#include <iostream>
#include <vector>
#include <regex>
#include <fstream>

using namespace std;

string regs[] = {"t1","t2","t3","t4","t5","t6","t7","t8","t9","s0","s1","s2","s3","s4","s5","s6","s7"};
vector<string> variables;
map<string,string> table;
map<string,int> reg_ok;


void save_var(string Inter)
{
    regex temp_re("temp\\d+");
    smatch result;
    
    string::const_iterator iter = Inter.begin();
    string::const_iterator iterEnd = Inter.end();
    
    while (regex_search(iter,iterEnd,result,temp_re))
    {
        string s = result[0];
        variables.emplace_back(s);
        iter = result[0].second;
    }
}

string load_int(const string& filename)
{
    string lines;
    ifstream in(filename);
    if(in)
    {
        string s;
        while (getline(in,s))
        {
            if (s == " ")
                continue;
            lines.append(s);
            lines.append("\n");
        }
    }
    in.close();
    return lines;
}

string reg_init(const string& temp_str)
{
    for (auto it = variables.begin(); it!=variables.end(); ++it)
        if (*it == temp_str)
        {
            it = variables.erase(it);
            break;
        }

    //如果已经存在寄存器分配，那么直接返回寄存器
    if (table.find(temp_str) != table.end())
        return "$"+table[temp_str];
    //没找到
    else
    {
        vector<string> keys;
        //已经分配寄存器的变量key
        for (auto & it : table)
            keys.emplace_back(it.first);

        //当遇到未分配寄存器的变量时，清空之前所有分配的临时变量的映射关系
        for (auto & key : keys)
            if (key.find("temp")!=string::npos && find(variables.begin(),variables.end(),key) == variables.end())
            {
                reg_ok[table[key]] = 1;
                table.erase(key);
            }

         //对于所有寄存器
        for (const auto & reg : regs)
            if(reg_ok[reg] == 1)    //如果寄存器可用
            {
                //将可用寄存器分配给该变量，映射关系存到table中
                table[temp_str] = reg;
                //寄存器reg设置为已用
                reg_ok[reg] = 0;
                return "$"+reg;
            }
    }
}

string translate(string temp_str){
    //将每行string按空格存成数组
    vector<string> line;
    string temp_res;
    stringstream input(temp_str);
    while (input>>temp_res)
        line.emplace_back(temp_res);
    //核心处理
    string temp_return;
    if(line[0] == "LABEL")
        return line[1]+":";
    if (line[1] == ":=") {
        if (line.size() == 3)
            if (temp_str[temp_str.length()-2] == '#')
                return "\tli " + reg_init(line[0]) + ","+line.back().back();
            else{
                temp_return = "\tmove ";
                temp_return += reg_init(line[0])+',';
                temp_return += reg_init(line[2]);
                return temp_return;
            }
        if (line.size() == 5){
            if (line[3] == "+")
                if (temp_str[temp_str.length()-2] == '#'){
                    temp_return = "\taddi ";
                    temp_return += reg_init(line[0])+",";
                    temp_return += reg_init(line[2])+",";
                    temp_return += line.back().back();
                    return temp_return;
                }
                else{
                    temp_return = "\tadd ";
                    temp_return += reg_init(line[0])+",";
                    temp_return += reg_init(line[2])+",";
                    temp_return += reg_init(line.back());
                    return temp_return;
                }
            else if (line[3] == "-"){
                if (temp_str[temp_str.length()-2] == '#'){
                    temp_return = "\taddi ";
                    temp_return += reg_init(line[0])+",";
                    temp_return += reg_init(line[2])+",";
                    temp_return += line.back().back();
                    return temp_return;
                }
                else{
                    temp_return = "\tsub ";
                    temp_return += reg_init(line[0])+",";
                    temp_return += reg_init(line[2])+",";
                    temp_return += reg_init(line.back());
                    return temp_return;
                }
            }
            else if (line[3] == "*"){
                temp_return = "\tmul ";
                temp_return += reg_init(line[0])+",";
                temp_return += reg_init(line[2])+",";
                temp_return += reg_init(line.back());
                return temp_return;
            }
            else if (line[3] == "/"){
                temp_return = "\tdiv ";
                temp_return += reg_init(line[2])+",";
                temp_return += reg_init(line.back()) + "\n\tmflo ";
                temp_return += reg_init(line[0]);
                return temp_return;
            }
            else if (line[3] == "<"){
                temp_return = "\tslt ";
                temp_return += reg_init(line[0])+",";
                temp_return += reg_init(line[2])+",";
                temp_return += reg_init(line.back());
                return temp_return;
            }
            else if (line[3] == ">"){
                temp_return = "\tslt ";
                temp_return += reg_init(line[0])+",";
                temp_return += reg_init(line.back())+",";
                temp_return += reg_init(line[2]);
                return temp_return;
            }
        }

        if (line[2] == "CALL")
            if (line[3] == "read" || line[3] == "print")
                return "\taddi $sp,$sp,-4\n\tsw $ra,0($sp)\n\tjal " + line.back() + "\n\tlw $ra,0($sp)\n\tmove " +reg_init(line[0]) + ",$v0\n\taddi $sp,$sp,4";
            else
                return "\taddi $sp,$sp,-24\n\tsw $t0,0($sp)\n\tsw $ra,4($sp)\n\tsw $t1,8($sp)\n\tsw $t2,12($sp)\n\tsw $t3,16($sp)\n\tsw $t4,20($sp)\n\tjal "+line.back()+"\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\tlw $t1,8($sp)\n\tlw $t2,12($sp)\n\tlw $t3,16($sp)\n\tlw $t4,20($sp)\n\taddi $sp,$sp,24\n\tmove "+reg_init(line[0])+" $v0";
    }
    if (line[0] == "GOTO")
        return "\tj "+line[1];
    if (line[0] == "RETURN")
        return "\tmove $v0,"+reg_init(line[1])+"\n\tjr $ra";
    if (line[0] == "IF") {
        if (line[2] == "=="){
            temp_return = "\tbeq ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "!="){
            temp_return = "\tbne ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == ">"){
            temp_return = "\tbgt ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "<"){
            temp_return = "\tblt ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == ">="){
            temp_return = "\tbge ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "<="){
            temp_return = "\tble ";
            temp_return += reg_init(line[1])+",";
            temp_return += reg_init(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
    }
    if (line[0] == "FUNCTION")
        return line[1]+":";
    if (line[0] == "CALL")
        if (line.back() == "read" || line.back() == "print")
            return "\taddi $sp,$sp,-4\n\tsw $ra,0($sp)\n\tjal "+line.back()+"\n\tlw $ra,0($sp)\n\taddi $sp,$sp,4";
        else
            return "\taddi $sp,$sp,-24\n\tsw $t0,0($sp)\n\tsw $ra,4($sp)\n\tsw $t1,8($sp)\n\tsw $t2,12($sp)\n\tsw $t3,16($sp)\n\tsw $t4,20($sp)\n\tjal "+line.back()+"\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\tlw $t1,8($sp)\n\tlw $t2,12($sp)\n\tlw $t3,16($sp)\n\tlw $t4,20($sp)\n\taddi $sp,$sp,24\n\tmove "+reg_init(line[0])+" $v0";
    if (line[0] == "ARG")
        return "\tmove $t0,$a0\n\tmove $a0,"+reg_init(line.back());
    if (line[0] == "PARAM")
        table[line.back()] = "a0";
    return " ";
}

void write_to_txt(const vector<string>& obj)
{
    ofstream out("./out.asm");
    string temp =".data\n"
                     "_prompt: .asciiz \"Enter an integer:\"\n"
                     "_ret: .asciiz \"\\n\"\n"
                     ".globl main\n"
                     ".text\n"
                     "read:\n"
                     "    li $v0,4\n"
                     "    la $a0,_prompt\n"
                     "    syscall\n"
                     "    li $v0,5\n"
                     "    syscall\n"
                     "    jr $ra\n"
                     "\n"
                     "write:\n"
                     "    li $v0,1\n"
                     "    syscall\n"
                     "    li $v0,4\n"
                     "    la $a0,_ret\n"
                     "    syscall\n"
                     "    move $v0,$0\n"
                     "    jr $ra\n";
                     "\n";
    out<<temp;
    for (auto & it:obj)
        out<<it<<endl;
    out.close();
}

int main()
{
    for (const auto & reg : regs) //初始化，所有寄存器都可用
        reg_ok[reg] = 1;

    string filename = "./test.txt";
    string Inter = load_int(filename);//读取中间代码

    //读取变量
    save_var(Inter);
    //翻译
    vector<string> obj;
    ifstream in(filename);
    string temp_line, obj_line;
    if(in) 
    {
        while (getline(in, temp_line)) 
        {
            obj_line = translate(temp_line);
            if (obj_line == " ")
                continue;
            obj.emplace_back(obj_line);
        }
    } 
    else
        cout<<"file open falied.\n";

    in.close();
    write_to_txt(obj);
    
    return 0;
}
