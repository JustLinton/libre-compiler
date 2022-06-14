#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>

using namespace std;


FILE *fp;
fstream output("tmp/mid.lex", ios::out | ios::trunc); // д�ļ�;

/**  
* COP compare operator �ȽϷ�
* AOP assignment operator ��ֵ��
* OOP operation operator ������
* EOP end operator ������
* SOP separate operator �ָ���
*/

// ����ؼ��֣���һ����ռλ������ʵ������
const string key[] = {
    "", "program", "const", "var", "procedure", "begin", "if", 
    "else", "end", "while", "call", "read", "write", "then", "odd", "do"
};

// �У���
int row, col;

bool isBC(char ch)
{
    if(ch == ' '){ // �ո�
        col++;
        return true;
    }
    else if(ch == '\t'){ // tab
        col += 4;
        return true;
    }
    else if(ch == '\r' || ch == '\n') // �س�
    {
        row++;
        col = 1;
        return true;
    }
    else{
        return false;
    }
}

void Concat(string& strToken, char ch) {
	strToken.push_back(ch);
}

bool IsLetter(char ch)
{
    if(ch >= 'a' && ch <= 'z'){
        return true;
    }
    else if(ch >= 'A' && ch <= 'Z'){
        return true;
    }
    else return false;
}

bool IsDigit(char ch)
{
    if(ch >= '0' && ch <= '9'){
        return true;
    }
    else return false;
}

int Reserve(string strToken)
{
	for(int i = 1; i <= 15; i++) {
		if (strToken == key[i]) {
			return i;
		}
	}
	return 0;
}

void Retract(char ch)
{
    if (ch != EOF){
        ungetc(ch, fp);
    }
}

int lexical_main(const char* input_file)
{
    fp = fopen(input_file, "r");
    if(!fp){
        cout << "�����ļ���ʧ��" << endl;
        exit(1);
    }

    if(!output.is_open())
    {
        cout << "����ļ���ʧ��" << endl;
        exit(2);
    }

    row = col = 1;
    string strToken;
    char ch;

    while(1)
    {
        ch = fgetc(fp);
        if(ch == EOF) break;

        //1.�ж��Ƿ�Ϊ�հ�
        if(isBC(ch)){
            strToken = "";
        }
        //2.�ж��Ƿ�Ϊ��ĸ
        else if(IsLetter(ch)){
            //�������ַ���������
            while(IsDigit(ch) || IsLetter(ch))
            {
                Concat(strToken, ch);
                col++;
                ch = fgetc(fp);
            }
            
            //�ж��ǹؼ��ֻ���ID
            if (Reserve(strToken)){ 
                output << strToken << " RESERVED " << row << ' ' << col << endl;
            }
			else{
                output << strToken << " id " << row << ' ' << col << endl;
            }

            strToken = "";
            Retract(ch);
        }
        //3.�ж��Ƿ�Ϊ����
        else if (IsDigit(ch)){
            //������������������
			while (IsDigit(ch)) {
				Concat(strToken, ch);
                col++;
				ch = fgetc(fp);
			}

            //�����ֿ�ͷ��ID��������
            if (IsLetter(ch)) {

                //���������ַ���
                while (IsLetter(ch) || IsDigit(ch)){
                    Concat(strToken, ch);
                    col++;
                    ch = fgetc(fp);
                }
				cout << "[Lexical ERROR]" << " [" << row << "," << col <<"] " << "Invalid ID: " << strToken << endl;
                output << strToken << " id " << row << ' ' << col << endl;
            } 
            else {
                
                output << strToken << " INT " << row << ' ' << col << endl;
            }

            Retract(ch);
            strToken = "";
        }
        //4.�ж��Ƿ�Ϊ�����ַ�
        else{
            if(ch == '=')
            {
                col++;
                output << ch << " COP " << row << ' ' << col << endl; 
            }
            else if(ch == '#')
            {
                col++;
                output << "<> COP " << row << ' ' << col << endl;
            }
            else if(ch == '<')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '>')
                {
                    col++;
                    output << "<> COP " << row << ' ' << col << endl;
                }
                else if(ch == '=')
                {
                    col++;
                    output << "<= COP " << row << ' ' << col << endl;
                }
                else
                {
                    output << "< COP " << row << ' ' << col << endl;
                    Retract(ch);
                }
            }
            else if(ch == '>')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '=')
                {
                    col++;
                    output << ">= COP " << row << ' ' << col << endl;
                }
                else
                {
                    output << "> COP " << row << ' ' << col << endl;
                    Retract(ch);
                }
            }
            else if(ch == ':')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '=')
                {
                    col++;
                    output << ":= AOP " << row << ' ' << col << endl; 
                }
                else
                {
                	cout << "[LEXICAL ERROR]" << " [" << row << "," << col << "] " << "Missing \"=\" near the \":\" " << endl;
                    output << ":= AOP " << row << ' ' << col << endl; 
					Retract(ch);
                }
            }
            else if(ch == '+' || ch == '-' || ch == '*' || ch == '/')
            {
                col++;
                output << ch << " OOP " << row << ' ' << col << endl; 
            }
            else if(ch == ';')
            {
                col++;
                output << ch << " EOP " << row << ' ' << col << endl; 
            }
            else if(ch == '(' || ch == ')' || ch == ',' || ch == '.')
            {
                col++;
                output << ch << " SOP " << row << ' ' << col << endl; 
            }
            else
            {
                col++;
                output << ch << " UNKNOWN " << row << ' ' << col << endl;
            }
        }
    }
    fclose(fp);
    output.close();


    return 0;
}
