#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<sstream>
#include<stack>
#include<bits/stdc++.h>

using namespace std;

const int MAX_N = 1e4;
string order[] = { "LIT","OPR","LOD","STO","CAL","INT","JMP","JPC","RED","WRT" };

#define CONST 0
#define VAR 1
#define PROCEDURE 2

#define LIT 0 
#define OPR 1
#define LOD 2
#define STO 3
#define CAL 4
#define INT 5
#define JMP 6
#define JPC 7
#define RED 8
#define WRT 9

class Unit{
public:
    string val;
    string key;
    int row;
    int col;

    void print()
    {
        cout << "-------------" << endl;
        cout << "Value: " << val << endl;
        cout << "Key: " << key << endl;
        cout << "Position: [" << row << ',' << col << "]" << endl;
        cout << "-------------" << endl;
    }
};

Unit unit, last_unit;//��ǰɨ�����һ��ɨ��
stack<int> errorStack;//��Ŵ���
bool error = false;//�����־

fstream in;//�����ļ�
fstream out;//����ļ�
string line;//�����һ������

fstream gtable_output("output/gtable.txt", ios::out | ios::trunc); // ���ű�
fstream target_output("output/target.s", ios::out | ios::trunc); // ���ű�

struct Pcode {  //Ŀ�����
	int f;//������
	int l;//��β�
	int a;//���ݲ�ͬ��ָ����������
}Pcode[MAX_N];

struct SymTable {  //���ű�
	string name;
	int type;//const=0, var=1, procedure=2
	int value;
	int level;
	int adr;
	int size;
	int num;    //�����ڳ�����ֵĴ����������ڴ��Ż�
}SymTable[MAX_N];

int tx = 0;// table���±�ָ��,���ű���ַ��1��ʼ
int dx = 0;// ����ÿ������������ջ����Ա����̻���ַ��ƫ����
int cx = 0;// ָ����һ����ַ
int lev = 0;
int dataStack[MAX_N]; //����ջ
int T;//ջ���Ĵ���
int B;//ջ��ַ�Ĵ���
int P;//����ָ��
int I;//ָ��Ĵ���
int INT_pos;// ��¼ָ��INT��λ��


//-----------��������----------------//
void test(int type);// ��ͬ��
void addError(int type, string name = "");
void ReadLine();
void Factor();
void Term();
void Exp();
void Lexp();
void Statement();
void Body();
void para_func();
void Proc();
void Block(); 
void Var();
void Vardecl();
void Const();
void Condecl();
void Prog();
void OpenFile();
void CloseFile();

void gen(int f, int l, int a);   //����p����
void addVar(string name, int level, int adr);    //���������뵽���ű�
void addConst(string name, int level, int val);   //���������뵽���ű�
void addProcedure(string name, int level, int adr);  //�����̵��뵽���ű�
int position(string name);  //�ڷ��ű���������Ϊname�ķ���
bool is_the_same_level(string name, int lev);   //�Ƿ���ͬ��
bool is_pre_level(string name, int lev); // 
int stringtoint(string str);
int findproc(int pos);
int getBase(int nowBp, int lev);
void printPcode();   //����м����
void printTable();  //������ű�
void interpreter();


//-----------��������----------------//

void gen(int f, int l, int a)
{
	Pcode[cx].f = f;
	Pcode[cx].l = l;
	Pcode[cx].a = a;
	cx++;
}

void addVar(string name, int level, int adr)
{
    tx++;
	SymTable[tx].type = VAR;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}

void addConst(string name, int level, int val)
{
    tx++;
	SymTable[tx].type = CONST;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].value = val;
	SymTable[tx].num = 1;
}

void addProcedure(string name, int level, int adr)
{
    tx++;
	SymTable[tx].type = PROCEDURE;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}

int position(string name)
{
    for(int i = tx; i > 0; i--)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
			return i;
	}
	for(int i = tx; i > 0; i--)
	{
		if (SymTable[i].name == name)
			return i;
	}
	return -1;
}

bool is_the_same_level(string name, int lev)//�Ƿ���ͬ��
{    

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level == lev)
		{
			return true;
		}
	}
	return false;
}

bool is_pre_level(string name, int lev)
{
    for(int i = 1; i <= tx; i++)
    {
        // cout << SymTable[i].name << "   " << SymTable[i].value << "   " << SymTable[i].level << endl;
        if(SymTable[i].name == name && SymTable[i].level <= lev)
            return true;
    }
    return false;
}

int stringtoint(string str)
{
    int res = 0;
    for(int i = 0; i < str.size(); i++)
    {
        res *= 10;
        res += str[i] - '0';
    }
    return res;
}

int findproc(int pos) // Ѱ�ҹ���
{
    for (int i = pos; i >= 1; i--)
	{
		if (SymTable[i].type == PROCEDURE)
		{
			return i;
		}
	}
	return -1;
}

int getBase(int nowBp, int lev)    //���ݲ��õ�����ַ
{ 
	int oldBp = nowBp;
	while(lev > 0)//�����ڲ��ʱѰ�ҷǾֲ�����
	{
		oldBp = dataStack[oldBp + 1];//ֱ�����Ļ��¼�׵�ַ
		lev--;
	}
	return oldBp;
}

void printPcode()   //����м����
{
    for (int i = 0; i < cx; i++)
    {    
    	// cout << i << " ";
		// printf("%s %d %d\n", order[Pcode[i].f].c_str(), Pcode[i].l, Pcode[i].a);//����.c_str()��ӡstring�ַ��� 
        target_output<<order[Pcode[i].f].c_str()<<'\t'<<Pcode[i].l<<'\t'<<Pcode[i].a<<endl;
	}
}

void printTable()  //������ű�
{
    int i = 1;
    gtable_output<< "N."<<'\t'<< "K."<<'\t'<< "V."<<'\t'<<"L."<<'\t'<<"A."<<endl;
    while(SymTable[i].num)
    {
		// printf("����:%s   ����:%d   ��ֵ:%d   ���:%d   ��Ե�ַ:%d    ���ִ���:%d\n",
        // SymTable[i].name.c_str(), SymTable[i].type, SymTable[i].value,SymTable[i].level, SymTable[i].adr, SymTable[i].num);
        gtable_output<< SymTable[i].name.c_str()<<'\t'<< SymTable[i].type<<'\t'<< SymTable[i].value<<'\t'<<SymTable[i].level<<'\t'<<SymTable[i].adr<<endl;
        i++;    
    }
}

void interpreter()
{
    //��ʼ��
    P = 0;//�����ַ�Ĵ���
	B = 0;//��ַ�Ĵ���
	T = 0;//ջ���Ĵ���
    int t;
    do
    {
        I = P;
        P++;
        switch (Pcode[I].f)//��ȡα������
        {
        case 0: //LIT 0 a��ȡ����a��������ջջ��
			dataStack[T] = Pcode[I].a;
			T++;
			break;
		case 1: //OPR 0 a��ִ�����㣬a��ʾִ��ĳ������
			switch (Pcode[I].a)
			{
			case 0:						//opr,0,0 ���ù��̽����󣬷��ص��õ㲢��ջ
				T = B;                  //�ָ�����ǰջ��
				P = dataStack[B + 2];	//���ص�ַ
				B = dataStack[B];		//��̬��
				break;
			case 1:                 //opr 0,1ȡ��ָ��
				dataStack[T - 1] = -dataStack[T - 1];
				break;
			case 2:                 //opr 0,2��ӣ���ԭ��������Ԫ����ȥ�����������ջ��
				dataStack[T - 2] = dataStack[T - 1] + dataStack[T - 2];
				T--;
				break;
			case 3:					//OPR 0,3 ��ջ����ȥջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 2] - dataStack[T - 1];
				T--;
				break;
			case 4:    				//OPR 0,4��ջ������ջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 2] * dataStack[T - 1];
				T--;
				break;
			case 5:					//OPR 0,5��ջ������ջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 2] / dataStack[T - 1];
				T--;
				break;
			case 6:                 //ջ��Ԫ��ֵ��ż�жϣ����ֵ��ջ,����Ϊ1
				dataStack[T - 1] = dataStack[T - 1] % 2;
				break;
			case 7:
				break;
			case 8:					//��ջ����ջ���Ƿ���ȣ�������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 1] == dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
				}
                else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 9:					//��ջ����ջ���Ƿ񲻵ȣ�������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 1] != dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 10:				//��ջ���Ƿ�С��ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] < dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 11:				//��ջ���Ƿ���ڵ���ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] >= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 12:				//��ջ���Ƿ����ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] > dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
                else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 13:				//��ջ���Ƿ�С�ڵ���ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] <= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 15:				//��Ļ�������
				cout << endl;
				break;
			}
			break;
        case 2: //LOD L ��a ȡ��������Ե�ַΪa�����ΪL���ŵ�����ջ��ջ��
			dataStack[T] = dataStack[Pcode[I].a + getBase(B, Pcode[I].l)];
//			cout << I << "------" << dataStack[T] << endl;
			T++;
			break;
        case 3: //STO L ��a ������ջջ�������ݴ����������Ե�ַΪa����β�ΪL��
            dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = dataStack[T - 1];
			T--;
			break;
        case 4: //CAL L ��a ���ù��̣�ת��ָ�����ڵ�ַΪa����β�ΪL��
			dataStack[T] = B;		//��̬����ֱ��������
			dataStack[T + 1] = getBase(B, Pcode[I].l);	//��̬��������ǰ���й���
			dataStack[T + 2] = P;		//���ص�ַ����һ��Ҫִ�е�
			B = T;
			P = Pcode[I].a;
			break;
        case 5: //INT 0 ��a ����ջջ��ָ������a
			T = B + Pcode[I].a;
			break;
		case 6: //JMP 0 ��a������ת�Ƶ���ַΪa��ָ��
			P = Pcode[I].a;
			break;
		case 7: //JPC 0 ��a ����ת��ָ�ת�Ƶ���ַΪa��ָ��
			if (dataStack[T - 1] == 0)
				P = Pcode[I].a;
			break;
		case 8: //RED L ��a �������ж���һ�����ݲ��������
            cout << "input prompt:";
			cin >> t;
			dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = t;
			break;
		case 9: //WRT 0 0 ջ��ֵ�������Ļ
			cout << dataStack[T - 1] << " ";
			break;
		}
	} while (P != 0);
}

void test(int type)
{
    if(type == 1)//statement��first��
    {
        while(unit.key != "id" && unit.val != "if" && unit.val != "while" && unit.val != "call" && unit.val != "begin" && unit.val != "read" && unit.val != "write")
        {
//            addError(17);//�˵��ʶ��࣬superfluous
            ReadLine();
        }
    }
    else if(type == 2)//block��first��
    {
        if(unit.val == "const" && unit.val == "var" && unit.val == "procedure")
            return;
        
        while(unit.val != "const" && unit.val != "var" && unit.val != "procedure" && unit.val != "begin" && unit.key != "id" && unit.val != "if" && unit.val != "while" && unit.val != "call" && unit.val != "read" && unit.val != "write")
        {
//            addError(17);//�˵��ʶ��࣬superfluous
            ReadLine();
        }

    }
    else
    {

    }
}

/**
 * add an error.
 * @param type Error type.
 */

void addError(int type, string name)//ȱʡ����
{
    errorStack.push(type);
    error = true;
    switch (type)
    {
    case 0://ƴд����
        printf("[Grammar error][%d,%d] Spell error \"program\"\n", unit.row, unit.col);
        break;
    case 1://program��ȱ�ٱ�ʶ��
        printf("[Grammar error][%d,%d] Missing identifier after \"program\"\n", unit.row, unit.col);
        break;
    case 2://ȱ�ٷֺ�
        printf("[Grammar error][%d,%d] Missing \";\" \n", last_unit.row, last_unit.col);
        break;
    case 3://ȱ�������� 
        printf("[Grammar error][%d,%d] Missing \")\" \n", unit.row, unit.col);
        break;
    case 4://ȱ�������� 
        printf("[Grammar error][%d,%d] Missing \"(\" \n", unit.row, unit.col);
        break;
    case 5://ȱ�ٶ��� 
        printf("[Grammar error][%d,%d] Missing \",\" \n", unit.row, unit.col);
        break;
    case 6://ȱ��id
        printf("[Grammar error][%d,%d] Missing identifier \n", unit.row, unit.col);
        break;
    case 7://ȱ�ٱȽϷ�
        printf("[Grammar error][%d,%d] Missing compare operator \n", unit.row, unit.col);
        break;
    case 8://ȱ�ٸ�ֵ�� 
        printf("[Grammar error][%d,%d] Missing assignment operator \n", unit.row, unit.col);
        break;
    case 9://ȱ��then
        printf("[Grammar error][%d,%d] Missing \"then\" \n", unit.row, unit.col);
        break;
    case 10://ȱ��do
        printf("[Grammar error][%d,%d] Missing \"do\" \n", unit.row, unit.col);
        break;
    case 11://ȱ��begin
        printf("[Grammar error][%d,%d] Missing \"begin\" \n", unit.row, unit.col);
        break;
    case 12://ȱ��end
        printf("[Grammar error][%d,%d] Missing \"end\" \n", unit.row, unit.col);
        break;
    case 13://������ֵ�ź���ӦΪ����
        printf("[Grammar error][%d,%d] after\":=\" should be a number  \n", unit.row, unit.col);
        break;
    case 14://ȱ��:
        printf("[Grammar error][%d,%d] Missing \":\" \n", unit.row, unit.col);
        break; 
    case 15://ȱ��:=
        printf("[Grammar error][%d,%d] Missing \":=\" \n", unit.row, unit.col);
        break;
    case 16://ȱ��program
        printf("[Grammar error][%d,%d] Missing \"program\" \n", unit.row, unit.col);
        break;
    case 17://���ڶ���ĵ���
        printf("[Grammar error][%d,%d] the word is superfluous \n", unit.row, unit.col);
        break;
    case 18://�ñ���δ����
        if(name != "")
        	printf("[error][%d,%d] not exist %s\n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] not exist %s\n", unit.row, unit.col, unit.val.c_str());
        break;
    case 19://���Ǳ���
    	if(name != "")
        	printf("[error][%d,%d] %s is not a variable \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a variable \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 20://���ǳ���
        if(name != "")
        	printf("[error][%d,%d] %s is not a const \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a const \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 21://���ǹ���
        if(name != "")
        	printf("[error][%d,%d] %s is not a procedure \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a procedure \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 22://����������ƥ��
        printf("[error][%d,%d] The number of parameters does not match \n", unit.row, unit.col);
        break;
    case 23://���ض���
        if(name != "")
        	printf("[error][%d,%d] Duplicate definition %s\n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] Duplicate definition %s\n", unit.row, unit.col, unit.val.c_str());
        break;

    default:
        printf("[error][%d,%d] Unknown error\n", unit.row, unit.col);
        break;
    }
}

/**
 * Read file line by line.
 * @return Global variables: unit.val & unit.key.
 */
void ReadLine()
{
    getline(in, line);
    last_unit = unit; 
    istringstream iss(line);//�ָ�ɵ���Ԫ��
    iss >> unit.val;//���ʵ�����
    iss >> unit.key;//���ʵ�����
    iss >> unit.row;
    iss >> unit.col;
}

/**
 * <factor>��<id>|<integer>|(<exp>)
 */
void Factor()//-------Ӧ��ok-----------
{
    if(unit.key == "id")
    {
        //�ж�id�Ƿ��Ѿ�����
        if(is_pre_level(unit.val, lev) == false)
            addError(18);
        else
        {
            int i = position(unit.val);
            SymTable[i].num++;

            if (SymTable[i].type == VAR)
			{
				gen(LOD, lev - SymTable[i].level, SymTable[i].adr);
			}
			else if (SymTable[i].type == CONST)
			{
				gen(LIT, 0, SymTable[i].value);
			}
			else
			{
				addError(19);//���Ǳ����ͳ���
				return;
			}
        }
        ReadLine();
    }
    else if(unit.key == "INT")
    {
        gen(LIT, 0, stringtoint(unit.val));
        ReadLine();
    }
    else if(unit.val == "(")
    {
        ReadLine();
        Exp();
        if(unit.val == ")")
            ReadLine();
        else
            addError(5);//ȱ��������
    }
    else
    {
        addError(6);//ȱ�ٱ�ʶ��
    }
}

/**
 * <term> �� <factor>{<mop><factor>}
 */
void Term()//-------Ӧ��ok-----------
{
    Factor();
    while(unit.val == "*" || unit.val == "/")
    {
        string name = unit.val;
        ReadLine();
        Factor();

        if(name == "*")
            gen(OPR, 0, 4);
        else if(name == "/")
            gen(OPR, 0, 5);
    }
}

/**
 * <exp> �� [+|-]<term>{<aop><term>}
 */
void Exp() //-------Ӧ��ok-----------
{
    string name;
    if(unit.val == "+" || unit.val == "-")
    {
        name = unit.val;//�ȼ�¼���������ȶ�����������������gen
        ReadLine();
    }    
    Term();
    if (name == "-")
	{
		gen(OPR, 0, 1);
	}
    while(unit.val == "+" || unit.val == "-")
    {
        name = unit.val;
        ReadLine();
        Term();
        if(name == "+")
            gen(OPR, 0, 2);
        else if(name == "-")
            gen(OPR, 0, 3);
        else
            exit(-1);//�����ܵ�������ط�
    }
}

/**
 * <lexp> �� <exp> <lop> <exp>|odd <exp>
 */
void Lexp()//-------Ӧ��ok-----------
{
    if(unit.val == "odd")
    {
        ReadLine();
        Exp();
        gen(OPR, 0, 6);
    }
    else
    {
        Exp();
        string key = unit.val;
        if(unit.key != "COP")
        {
		    addError(7);//ȱ�ٱȽϷ�
		    Exp();
		}
        else
        {
        	ReadLine();
            Exp();
        }

        if(key == "=")
            gen(OPR, 0, 8);
        else if(key == "<>")
            gen(OPR, 0, 9);
        else if(key == "<")
            gen(OPR, 0, 10);
        else if(key == "<=")
            gen(OPR, 0, 13);
        else if(key == ">")
            gen(OPR, 0, 12);
        else if(key == ">=")
            gen(OPR, 0, 11);
    }
}

/**
 * <statement> �� <id> := <exp>
               |if <lexp> then <statement>[else <statement>]
               |while <lexp> do <statement>
               |call <id>[��<exp>{,<exp>}��]
               |<body>
               |read (<id>{��<id>})
               |write (<exp>{,<exp>})
 */
void Statement()
{
    if(unit.key == "id")
    {
        string name = unit.val;
        ReadLine();
        if(unit.val == ":=")
        {
            ReadLine();
        }
        else
        {
            addError(8);//ȱ�ٸ�ֵ��
        }
        int i = position(name);
        SymTable[i].num++;
        Exp();

        if(is_pre_level(name, lev) == false)
            addError(18, name);//δ����
        else
        {
            if(SymTable[i].type == VAR)
                gen(STO, lev - SymTable[i].level, SymTable[i].adr);
            else
                addError(19,name);//���Ǳ���
        }
    }

    else if(unit.val == "if")
    {
        ReadLine();
        Lexp();
        int cx1;
        if(unit.val == "then")
        {
            cx1 = cx;
            gen(JPC, 0, 0);
            ReadLine();
        }    
        else
            addError(9);//ȱ��then
        
        Statement();
        int cx2 = cx;
        gen(JMP, 0, 0);//��jmp����jpc��������

        //����
        Pcode[cx2].a = cx;
        Pcode[cx1].a = cx;
        if(unit.val == "else")
        {
            ReadLine();
            Statement();
            Pcode[cx2].a = cx;//����
        }        
    }
    else if(unit.val == "while")
    {
		int cx1 = cx;
        ReadLine();
        Lexp();

        int cx2 = cx;
		gen(JPC, 0, 0);
        if(unit.val == "do")
            ReadLine();
        else
            addError(10);//ȱ��do
            
        Statement();
        gen(JMP, 0, cx1);
		Pcode[cx2].a = cx;
    }
    else if(unit.val == "call")
    {
        ReadLine();
        int i;
        int count = 0;//�����ĸ���
        if(unit.key == "id")
        {
            if(is_pre_level(unit.val, lev) == false)
            {
                i = position(unit.val);
                SymTable[i].num++;
                addError(18);//δ���� 
            }
            else
            {
                i = position(unit.val);
                SymTable[i].num++;
                if(SymTable[i].type != PROCEDURE)
                    addError(21);//���ǹ���
            }
            ReadLine();
        }
        else
            addError(6);//ȱ��id

        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//ȱ��(
        
        if (unit.val == ")")//˵���޲���
		{
			gen(CAL, lev - SymTable[i].level, SymTable[i].value);//SymTable[i].value Ҫ��ת�Ĺ���ָ���ַ
		}
        if(unit.val == "+" || unit.val == "-" || unit.key == "id" || unit.key == "INT" || unit.val == "(")
        {
            Exp();
            count++;
            while(unit.val == ",")
            {
                ReadLine();
                Exp();
                count++;
            }
            if (count != SymTable[i].size)
                addError(22);//����������ƥ��
//            cout << "i:" << i << endl;
            gen(CAL, lev - SymTable[i].level, SymTable[i].value);//SymTable[i].value Ҫ��ת�Ĺ���ָ���ַ
        }
        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//ȱ��)
    }
    else if(unit.val == "begin")
    {
        Body();
    }
    else if(unit.val == "read")
    {
        ReadLine();
        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//ȱ��(

        if(unit.key == "id")
        {
            if(is_pre_level(unit.val, lev) == false)
            {
                addError(18);//δ����
            }
            else
            {
                int i = position(unit.val);
                SymTable[i].num++;
                if(SymTable[i].type == VAR)
                {
                    gen(RED, lev - SymTable[i].level, SymTable[i].adr);
                }
                else
                    addError(19);//���Ǳ���
            }
            ReadLine();
        }
        else
            addError(6);//ȱ��id
        
        while(unit.val == ",")
        {
            ReadLine();
            if(unit.key != "id")
            {
                addError(6);//ȱ��id
                if(unit.val != ")" && unit.val != ";")
                	ReadLine();
                break;
            }

            if(is_pre_level(unit.val, lev) == false)
                addError(18);//δ����
            else
            {
                int i = position(unit.val);
                SymTable[i].num++;
                
                if(SymTable[i].type == VAR)
                    gen(RED, lev - SymTable[i].level, SymTable[i].adr);
                else addError(19);//���Ǳ���
            }
            ReadLine();
        }

        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//ȱ��)
    }
    else if(unit.val == "write")
    {
        ReadLine();
        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//ȱ��(
        
        Exp();
        gen(WRT, 0, 0);
        while(unit.val == ",")
        {
            ReadLine();
            Exp();
            gen(WRT, 0, 0);
        }
        gen(OPR, 0, 15);//�������
        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//ȱ��)
    }
    else
    {
        cout << "����������statement������ط�" << endl;
    }
}

/**
 * <body> �� begin <statement>{;<statement>}end
 */
void Body()//-------Ӧ��ok-----------
{
    if(unit.val == "begin")
        ReadLine();
    else
        addError(11);//ȱ��begin
    
    //ͬ��
    test(1);
    Statement();

    while(unit.val == ";")
    {
        ReadLine();
        test(1);

        Statement();
    }
    if(unit.val == "end")
        ReadLine();
    else
        addError(12);//ȱ��end

}

void para_func()//-------Ӧ��ok-----------
{
    if(unit.key == "id")
    {
        ReadLine();
        while(unit.val == "," || unit.key == "id")
        {
            if(unit.val == ",")
                ReadLine();
            else
                addError(5);//ȱ��,
            
            if(unit.key == "id")
            {
                ReadLine();
            }
            else
            {
                addError(6);//ȱ��id
                while(unit.val != ")" && unit.key != "id" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                    ReadLine();
                if(unit.key == "id")
                    ReadLine();
            }
        }
        if(unit.val != ")")
        {   
            addError(3);//ȱ��)
        }
        else
            ReadLine();
    }
    else if(unit.val == ")")
        ReadLine();
    else
    {
        addError(3);//ȱ��)
        ReadLine();
        while(unit.val != ")" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
            ReadLine();
    }
}

/**
 * <proc> �� procedure <id>��[<id>{,<id>}]��;<block>{;<proc>}
 */
void Proc()//-------Ӧ��ok-----------
{
    //����ú�����ǰ������procedure���������ж�procedure
    
    int count = 0;
    int tx0;
    ReadLine();
    if(unit.key == "id")
    {
        if(is_the_same_level(unit.val, lev))
        {
            addError(23);//���ض���
        }
        tx0 = tx + 1;
        addProcedure(unit.val, lev, dx);
        lev++;
        ReadLine();

        if(unit.val != "(")
        {
            addError(4);//ȱ��(
        }
        else
        {
            ReadLine();
        }
        if(unit.key == "id")
        {
            string name = unit.val;
            addVar(name, lev, count + 3);
            count++;
            SymTable[tx0].size = count;

            ReadLine();
            while(unit.val == "," || unit.key == "id")
            {
                if(unit.val == ",")
                    ReadLine();
                else
                    addError(5);//ȱ��,
                
                if(unit.key == "id")
                {
                    string name = unit.val;
                    addVar(name, lev, count + 3);
                    count++;
                    SymTable[tx0].size = count;
                    ReadLine();
                }
                else
                {
                    addError(6);//ȱ��id
                    while(unit.val != ")" && unit.key != "id" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                        ReadLine();
                    if(unit.key == "id")
                        ReadLine();
                }
            }
            if(unit.val != ")")
            {   
                addError(3);//ȱ��)
            }
            else
                ReadLine();
        }
        else if(unit.val == ")")
            ReadLine();
        else
        {
            addError(3);//ȱ��)
            ReadLine();
            while(unit.val != ")" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                ReadLine();
        }
    }
    else
    {
        addError(6);//ȱ��id
        while(unit.val != "(" && unit.val != ")" && unit.val != ";")
            ReadLine();
        if(unit.val == "(")
        {
            ReadLine();
            para_func();
        }
    }

    if(unit.val == ";")
        ReadLine();
    else
    {
        addError(2);//ȱ��;
        test(2);//ͬ��
    }

    if(unit.val == "const" || unit.val == "var" || unit.val == "procedure" || unit.val == "begin" || unit.key == "id" || unit.val == "if" || unit.val == "while" || unit.val == "call" || unit.val == "read" || unit.val == "write")
        Block();

    while(unit.val == ";")
    {
        ReadLine();
        Proc();
    }
    
}

void Var()//-------Ӧ��ok-----------
{
    if(unit.key == "id")
    {
        string name = unit.val;
		if (is_the_same_level(name, lev))
		{
			addError(23);//�ظ�����
		}

		addVar(name, lev, dx);
		dx++;
        ReadLine();
    }
    else
    {
        addError(6);//var����ӦΪ��ʶ��
        if(unit.key == "INT")
            ReadLine();
        while(unit.val != "," && unit.key != "id" && unit.val != ";")
            ReadLine();
    }
}

/**
 * <vardecl> �� var <id>{,<id>};
 * <id> �� l{l|d}
 * l represent letter.
 */
void Vardecl()//-------Ӧ��ok-----------
{
    ReadLine();
    Var();
    while(unit.val == "," || unit.key == "id")
    {
        if(unit.key == "id")
        {
            string name = unit.val;
            if (is_the_same_level(name, lev))
            {
                addError(23);//�ظ�����
            }

            addVar(name, lev, dx);
            dx++;
            addError(5);//ȱ��,
        }    
        else
        {
        	ReadLine();
        	Var();
		}    
    }

    if(unit.val == ";")
        ReadLine();
    else
        addError(2);//ȱ��;

}

/**
 * Const variables declaration.
 * <const> �� <id>:=<integer>
 * <id> �� l{l|d}
 * l represent letter.
 */
void Const()//-------Ӧ��ok-----------
{
    ReadLine();
    string name;
    if(unit.key == "id")
    {
        name = unit.val;
        ReadLine();
    }
    else
    {
        addError(6);//ȱ��id
        while(unit.val != ":=" && unit.val != "=" && unit.val != ";")
            ReadLine();
        if(unit.val == ";")
            return;
    }

    if(unit.val == ":=")
    {
        ReadLine();
        if(unit.key == "INT")
        {
            int value = stringtoint(unit.val);

            if(is_the_same_level(name, lev))
                addError(23);//���ض���
            addConst(name, lev, value);
			ReadLine();
		}    
        else
        {
            addError(13);//������ֵ�ź���ӦΪ����
            if(unit.key == "id")
                ReadLine();
        }
    }
    else
    {
        if(unit.val == "=")
        {
            addError(14);//ȱ��:
            ReadLine();

            if(unit.key == "INT")
            {
                int value = stringtoint(unit.val);

                if(is_the_same_level(name, lev))
                    addError(23);//���ض���
                addConst(name, lev, value);
                ReadLine();
            } 
            else
                addError(13);//��ֵ�ź���ӦΪ����
        }
        else
        {
            addError(15);//ȱ��:=
            while(unit.val != ":=" && unit.key != "INT" && unit.val != ",")
                ReadLine();
            if(unit.key != "INT")
                return;
        }
    }

}

/**
 * Const variables declaration.
 * <condecl> �� const <const>{,<const>};
 */
void Condecl()//-------Ӧ��ok-----------
{
    Const();
    bool flag = false;
    while(unit.val == "," || unit.key == "id")
    {
        flag = true;
        if(unit.key == "id")
            addError(5);//ȱ�ٶ��� 
        else
            ;//����Readline(),��Ϊ��Const�����ڵ�����Readline
        Condecl();
    }
    
    if(flag == false)
    {
        if(unit.val == ";")
            ReadLine();
        else
            addError(2);//ȱ��;
    }
}

/**
 * <block> �� [<condecl>][<vardecl>][<proc>]<body>
 */
void Block()//-------Ӧ��ok-----------
{
    int dx0 = dx;
	int tx0 = tx + 1;
	int n = 0;

    if (tx0 > 1)
	{
		n = findproc(tx0);
		tx0 -= SymTable[n].size;//��ȥ�βθ���
        dx = 3 + SymTable[n].size;
	}
	else
		dx = 3;

	int cx0 = cx;//��¼��תָ���λ��
	gen(JMP, 0, 0);

    if(unit.val == "const")
        Condecl();
    if(unit.val == "var")
        Vardecl();
    if(unit.val == "procedure")
    {
        Proc();
        lev--;
    }

    if(tx0 > 1)
    {
        n = findproc(tx0);
        for(int i = 0; i < SymTable[n].size; i++)
        {
            gen(STO, 0, SymTable[n].size + 3 - 1 - i);//��ʵ�ε�ֵ�����β�
        }
    }

    Pcode[cx0].a = cx;//����
    INT_pos = cx;//��¼�ȴ������λ��
    gen(INT, 0, dx);//����dx���ռ�

    if(tx != 1)
    {
//        cout << "n:" <<n<< endl;
        SymTable[n].value = cx - 1 - SymTable[n].size;  //������ڵ�ַ, ������SymTable[n].size��ָ��
//        cout << "SymTable[n].value:" << SymTable[n].value << endl;
    }

    Body();
    gen(OPR, 0, 0);

    tx = tx0;   // clear Sym
	dx = dx0;
}

/**
 * <prog> �� program <id>; <block>
 */
void Prog()//-------Ӧ��ok-----------
{
    ReadLine();
    if(unit.val != "program")
    {
        addError(16);//ȱ��program
        if(unit.key != "id" && unit.val != ";")
            ReadLine();
    }
    else
    {
        ReadLine();
    }

    if(unit.val != ";")
    {
        if(unit.key != "id")
        {
            ReadLine();
            addError(6);//ȱ��id
        }
        else
        {
            ReadLine();            
            if(unit.val != ";")
                addError(2);//ȱ��;
            else
                ReadLine();
        }
    }
    else
        addError(6);//ȱ��id
    
    while(unit.val != "const" && unit.val != "var" && unit.val != "procedure" && unit.val != "begin" && unit.val != ";")
    {
//        addError(17);//���ڶ���ĵ���
        ReadLine();
    }
    if(unit.val == ";")
        ReadLine();
    Block();
}

/**
 * Open IO files.
 */
void OpenFile()
{
    in.open("tmp/mid.lex", ios::in);
//    out.open("ga_output.txt", ios::out);
    if(!in)
    {
        cout << "�����ļ���ʧ��" << endl;
        exit(-1);
    }
//    if(!out)
//    {
//        cout << "����ļ���ʧ��" << endl;
//        exit(-1);
//    }
}

/**
 * Close IO files.
 */
void CloseFile()
{
    in.close();
    out.close();
    gtable_output.close();
    target_output.close();
}

int semantic_main()
{
//	LA();
    OpenFile();
	Prog();

    if(!gtable_output.is_open()||!target_output.is_open())
    {
        cout << "failed to output table or target." << endl;
        exit(2);
    }
	
    int count = 0; //��¼δ��ʹ�õı�������
	int i = 1;
    while (SymTable[i].num && !SymTable[i].level)  //�ڷ��ű��б�����0���������ķ���
	{
		if (SymTable[i].num == 1 && SymTable[i].type == VAR) //����Ǳ�����ֻ�����һ�Σ�����û��ʹ�ã����к����Ż�
		{
			int j = i + 1;                                      
			while (SymTable[j].num && !SymTable[j].level)    //�������ж���ı�����Ե�ַ��Ҫ��һ
			{
				SymTable[j].adr--;                  
				j++;
			}
			count++;    //δ��ʹ�õı���������һ
		}
		i++;
	}

	Pcode[INT_pos].a -= count;     //����INTָ��ٵĿռ�����һ��ɨ��֮�󣬽�����֮ǰ

    printPcode();
    printTable();

	// cout << "�Ƿ�����м����:1 or 0" << endl;
	// int flag;
	// cin >> flag;
	// if (flag)
	// {
	// 	printPcode();
	// }
	// cout << "�Ƿ�������ű�:1 or 0" << endl;
	// cin >> flag;
	// if (flag)
	// {
	// 	printTable();
	// 	cout << endl;
	// }

    
	if(!error) 
    	interpreter(); 
    else
    {
    	cout << "Failed to compile. Can not excute the target program." << endl; 
	}
    CloseFile();
	return 0;
}
