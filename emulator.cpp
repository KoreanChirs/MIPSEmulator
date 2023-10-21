#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <sstream>

using namespace std;

vector<string> instruction_vector;
vector<string> data_vector;
vector<string> stack_;
string register_value[32];
unordered_map<unsigned long long, string> data_memory;
unsigned long long data_start_address = 0x10000000;
unsigned long long instruction_start_address = 0x400000;
unsigned long long PC = 0x400000;

string print_memory;
unsigned long add_1;
unsigned long add_2;
bool is_m = false;
bool is_d = false;
int how_many_instruction_excute = 0;

string bin_to_hex(string binary_str){
    bitset<32> bin(binary_str);
    unsigned long hex_value = bin.to_ulong();
    stringstream st;
    st << hex << hex_value;
    string hex_ans = string("0x") + st.str();
    return hex_ans;
}

string hex_to_binary(string hexa){
    unsigned long long convert_int = stoul(hexa, nullptr, 16);
    bitset<32> binary_value(convert_int);
    return binary_value.to_string();
}
string hex_to_8_binary(string hexa){
    unsigned long long convert_int = stoul(hexa, nullptr, 16);
    bitset<8> binary_value(convert_int);
    return binary_value.to_string();
}

unsigned long long hex_to_int(string hexa){
    return stoul(hexa, nullptr, 16);
}

int main(int argc, char*argv[]){
    string input_file;
    for (int i = 1; i < argc; i++){
        string arg = argv[i];
        if (arg == "-m"){
            is_m = true;
            print_memory = argv[i + 1];
            size_t colon_pos = print_memory.find(':');
            string add1 = print_memory.substr(0,colon_pos);
            string add2 = print_memory.substr(colon_pos+1);
            add_1 = stoul(add1, nullptr, 16);
            add_2 = stoul(add2, nullptr, 16);
            if(add_1 > add_2){
                throw string("please enter the valid address");
            }
            i++;
        }
        else if(arg == "-d"){
            is_d = true;
        }
        else if(arg == "-n"){
            how_many_instruction_excute = stoi(argv[i + 1]);
            i++;
        }
        else{
            input_file = arg;
        }
    }
    ifstream binary_file(input_file);
    string line;
    int read_cnt = 0;
    int insturction_len, data_len;
    int data_cnt = 0;
    int instruction_cnt = 0;
    for(int i = 0; i < 32; i++){
        register_value[i] = string("0x0");
    }
    while(getline(binary_file,line)){
        if(read_cnt == 0){
            insturction_len = hex_to_int(line)/4;
        }
        else if(read_cnt == 1){
            data_len = hex_to_int(line)/4;
        }
        else{
            if(insturction_len > 0){
                instruction_vector.push_back(hex_to_binary(line));
                string bin = hex_to_binary(line);
                string first = bin.substr(0,8);
                string second = bin.substr(8,8);
                string third = bin.substr(16,8);
                string four = bin.substr(24,8);
                data_memory[instruction_start_address + 4*instruction_cnt] =  bin_to_hex(first);
                data_memory[instruction_start_address + 4*instruction_cnt + 1] =  bin_to_hex(second);
                data_memory[instruction_start_address + 4*instruction_cnt + 2] =  bin_to_hex(third);
                data_memory[instruction_start_address + 4*instruction_cnt + 3] =  bin_to_hex(four);
                insturction_len--;
                instruction_cnt++;
            }
            else{
                data_vector.push_back(hex_to_binary(line));
                string bin = hex_to_binary(line);
                string first = bin.substr(0,8);
                string second = bin.substr(8,8);
                string third = bin.substr(16,8);
                string four = bin.substr(24,8);
                data_memory[data_start_address + 4*data_cnt] =  bin_to_hex(first);
                data_memory[data_start_address + 4*data_cnt + 1] = bin_to_hex(second);
                data_memory[data_start_address + 4*data_cnt + 2] = bin_to_hex(third);
                data_memory[data_start_address + 4*data_cnt + 3] = bin_to_hex(four);
                data_len--;
                data_cnt++;
            }
        }
        read_cnt++;
    }
    PC = instruction_start_address;
    for(int i = 0; i < instruction_vector.size();){
        if(how_many_instruction_excute-- == 0){
            break;
        }
        string instruc = instruction_vector[i];
        string opcode = instruc.substr(0,6);
        if(opcode == string("000000")){ //R(ADDU, AND, JR, NOR,OR,SLTU,SLL,SRL,SUBU)
            string funct = instruc.substr(26,6);
            string rs = instruc.substr(6,5);
            string rt = instruc.substr(11,5);
            string rd = instruc.substr(16,5);
            bitset<32> rs_bitset(rs);
            bitset<32> rt_bitset(rt);
            bitset<32> rd_bitset(rd);
            unsigned long rs_index = rs_bitset.to_ulong();
            unsigned long rt_index = rt_bitset.to_ulong();
            unsigned long rd_index = rd_bitset.to_ulong();
            bitset<32> rs_value(stoul(register_value[rs_index],nullptr,16));
            bitset<32> rt_value(stoul(register_value[rt_index],nullptr,16)); 

            if(funct == string("100001")){ // ADDU //
                bitset<32> rd_real(rs_value.to_ulong() + rt_value.to_ulong());
                register_value[rd_index] = bin_to_hex(rd_real.to_string());
            }
            else if(funct == string("100100")){ //AND//
                bitset<32> ans = rs_value & rt_value;
                register_value[rd_index] = bin_to_hex(ans.to_string());
            }
            else if(funct == string("001000")){ // JR//
                unsigned long target_address = rs_value.to_ulong();
                i = (target_address-instruction_start_address)/4 - 1;
            }
            else if(funct == string("100111")){ // NOR//
                bitset<32> ans = ~(rs_value | rt_value);
                register_value[rd_index] = bin_to_hex(ans.to_string());
            }
            else if(funct == string("100101")){ // OR//
                bitset<32> ans = (rs_value | rt_value);
                register_value[rd_index] = bin_to_hex(ans.to_string());
            }
            else if(funct == string("101011")){ // SLTU//
                unsigned long rs_u = rs_value.to_ulong();
                unsigned long rt_u = rt_value.to_ulong();
                if(rs_u < rt_u){
                    register_value[rd_index] = string("0x1");
                }
                else{
                    register_value[rd_index] = string("0x0");
                }
            }
            else if(funct == string("000000")){ // SLL//
                string shamt = instruc.substr(21,5);
                bitset<32> binary_shamt(shamt);
                unsigned long shift_amt = binary_shamt.to_ulong();
                bitset<32> shift_left_rt = rt_value << shift_amt;
                register_value[rd_index] = bin_to_hex(shift_left_rt.to_string());
            }
            else if(funct == string("000010")){ // SRL//
                string shamt = instruc.substr(21,5);
                bitset<32> binary_shamt(shamt);
                unsigned long shift_amt = binary_shamt.to_ulong();
                bitset<32> shift_left_rt = rt_value >> shift_amt;
                register_value[rd_index] = bin_to_hex(shift_left_rt.to_string());
            }
            else if(funct == string("100011")){ // SUBU //
                long long rs_u = rs_value.to_ulong();
                long long rt_u = rt_value.to_ulong();
                long long ans = rs_u - rt_u;
                bitset<32> answer(ans);
                register_value[rd_index] = bin_to_hex(answer.to_string());
            }

        }
        if(opcode == string("001001") || opcode == string("001100") || opcode == string("000100") || opcode == string("000101") || opcode == string("001111") || opcode == string("100011") || opcode == string("001101") || opcode == string("100000") || opcode == string("001011") || opcode == string("101011") || opcode == string("101000")){
                string rs = instruc.substr(6,5);
                string rt = instruc.substr(11,5);
                bitset<32> rs_bitset(rs);
                bitset<32> rt_bitset(rt);
                unsigned long rs_index = rs_bitset.to_ulong();
                unsigned long rt_index = rt_bitset.to_ulong();
                bitset<32> rs_value(stoul(register_value[rs_index],nullptr,16));
                string imm = instruc.substr(16,16);
                string extend_imm;
                if(imm.substr(0,1) == string("1")){
                    extend_imm = string("1111111111111111") + imm;
                }
                else{
                    extend_imm = string("0000000000000000") + imm;
                }
                bitset<32> extend_imm_bin(extend_imm);
                long long sign_imm = static_cast<int>(extend_imm_bin.to_ulong());
            if(opcode == string("001001")){ //addiu // 
                unsigned long rs_u = rs_value.to_ulong();
                long long rs_l = rs_u;
                long long ans = sign_imm + rs_l;
                bitset<32> sum(ans);
                register_value[rt_index] = bin_to_hex(sum.to_string());
            }
            
            else if(opcode == string("001100")){ // ANDI //
                string imm = instruc.substr(16,16);
                bitset<32> imm_bin(imm);
                bitset<32> ans = rs_value & imm_bin;
                register_value[rt_index] = bin_to_hex(ans.to_string());
            }
            else if(opcode == string("000100")){ // BEQ //
                bitset<32> rt_value(stoul(register_value[rt_index],nullptr,16));
                if(rs_value.to_ulong() == rt_value.to_ulong()){
                    unsigned long target_add = PC + sign_imm*4 + 4;
                    i = (target_add-instruction_start_address)/4 - 1;
                }
            } 
            else if(opcode == string("000101")){ // BNE //
                bitset<32> rt_value(stoul(register_value[rt_index],nullptr,16));
                if(rs_value.to_ulong() != rt_value.to_ulong()){
                    unsigned long target_add = PC + sign_imm*4 + 4;
                    i = (target_add-instruction_start_address)/4 - 1;
                }
            }
            else if(opcode == string("001111")){ // LUI //
                string imm = instruc.substr(16,16);
                string ans = imm + string("0000000000000000");
                register_value[rt_index] = bin_to_hex(ans);
            }
            else if(opcode == string("100011")){ // LW
                long long rs_l = rs_value.to_ulong();
                long long target_add = rs_l + sign_imm;
                string first = hex_to_8_binary(data_memory[target_add]);
                string second = hex_to_8_binary(data_memory[target_add + 1]);
                string third = hex_to_8_binary(data_memory[target_add + 2]);
                string four = hex_to_8_binary(data_memory[target_add + 3]);
                string ans = first + second + third + four;
                register_value[rt_index] = bin_to_hex(ans);
            }
            else if(opcode == string("001101")){ // ORI //
                string imm = instruc.substr(16,16);
                bitset<32> imm_bin(imm);
                bitset<32> ans = rs_value | imm_bin;
                register_value[rt_index] = bin_to_hex(ans.to_string());
            }
            else if(opcode == string("100000")){ // LB //
                long long rs_l = rs_value.to_ulong();
                long long target_add = rs_l + sign_imm;
                string first = hex_to_8_binary(data_memory[target_add]);
                string extended_need;
                if(first.substr(0,1) == string("1")){
                    extended_need = string("111111111111111111111111") + first;
                }
                else{
                    extended_need = string("000000000000000000000000") + first;
                }
                bitset<32> ans(extended_need);
                register_value[rt_index] = bin_to_hex(ans.to_string());
            }
            else if(opcode == string("001011")){ // SLTIU // 
                long long rs_i = rs_value.to_ulong();
                if(rs_i < sign_imm){
                    register_value[rt_index] = string("0x1");
                }
                else{
                    register_value[rt_index] = string("0x0");
                }

            }
            else if(opcode == string("101011")){ // SW //
                long long rs_l = rs_value.to_ulong();
                long long target_add = rs_l + sign_imm;
                bitset<32> rt_value(stoul(register_value[rt_index],nullptr,16));
                string rt_str = rt_value.to_string();
                string first = rt_str.substr(0,8);
                string second = rt_str.substr(8,16);
                string third = rt_str.substr(16,24);
                string four = rt_str.substr(24,32);
                data_memory[target_add] = bin_to_hex(first);
                data_memory[target_add + 1] = bin_to_hex(second);
                data_memory[target_add + 2] = bin_to_hex(third);
                data_memory[target_add + 3] = bin_to_hex(four);
            }
            else if(opcode == string("101000")){ // SB 
                long long rs_l = rs_value.to_ulong();
                long long target_add = rs_l + sign_imm;
                bitset<32> rt_value(stoul(register_value[rt_index],nullptr,16));
                string rt_str = rt_value.to_string();
                string real_rt = rt_str.substr(24,8);
                data_memory[target_add] = bin_to_hex(real_rt);
            }
        }
        if(opcode == string("000010") || opcode == string("000011")){
            string im = instruc.substr(6,26);
            bitset<32> imm(im);
            unsigned long target = imm.to_ulong();
            unsigned long target_address = target * 4;
            if(opcode == string("000010")){ // J
                i = (target_address-instruction_start_address)/4 - 1;
            }
            else if(opcode == string("000011")){ // JAL
                bitset<32> ans(PC+4);
                register_value[31] = bin_to_hex(ans.to_string());
                i = (target_address-instruction_start_address)/4 - 1;
            }
        }
        i++;
        PC = instruction_start_address + i*4;
        if(is_d){
            cout << "Current register values:" << endl;
            cout << "------------------------------------"<< endl;
            cout << "PC: 0x" << hex << PC << endl;
            cout << "Registers:" << endl;
            for(int k = 0; k < 32; k++){
                cout << 'R' << to_string(k) << ": " << register_value[k] << endl;
            }
            if(is_m){
                cout << "Memory content [" << hex << add_1 <<".."<< hex << add_2 << "]:" << endl;
                cout << "------------------------------------" << endl;
                for(unsigned long t = add_1; t <= add_2; t++){
                    string fir_, second_, third_, last_;
                    if(data_memory.count(t) > 0){
                        fir_ = hex_to_8_binary(data_memory[t]);
                    }
                    else{
                        fir_ = hex_to_8_binary(string("0x0"));
                    }
                    if(data_memory.count(t+1) > 0){
                        second_ = hex_to_8_binary(data_memory[t + 1]);
                    }
                    else{
                        second_ = hex_to_8_binary(string("0x0"));
                    }
                    if(data_memory.count(t+2) > 0){
                        third_ = hex_to_8_binary(data_memory[t + 2]);
                    }
                    else{
                        third_ =  hex_to_8_binary(string("0x0"));
                    }
                    if(data_memory.count(t+3) > 0){
                        last_ = hex_to_8_binary(data_memory[t + 3]);
                    }
                    else{
                        last_ = hex_to_8_binary(string("0x0"));
                    }
                    string entire = fir_ + second_ + third_ + last_;
                    string hexx_ = bin_to_hex(entire);
                    cout << hex << t <<": " << hexx_ << endl;
                    t = t + 3;
                }
            }
        }
    }
    if((!is_d) || (how_many_instruction_excute == 0)){
        cout << "Current register values:" << endl;
        cout << "------------------------------------"<< endl;
        cout << "PC: 0x" << hex << PC << endl;
        cout << "Registers:" << endl;
        for(int k = 0; k < 32; k++){
            cout << 'R' << to_string(k) << ": " << register_value[k] << endl;
        }
        if(is_m){
            cout << "Memory content [" << hex << add_1 <<".."<< hex << add_2 << "]:" << endl;
            cout << "------------------------------------" << endl;
            for(unsigned long t = add_1; t <= add_2; t++){
                string fir_, second_, third_, last_;
                if(data_memory.count(t) > 0){
                    fir_ = hex_to_8_binary(data_memory[t]);
                }
                else{
                    fir_ = hex_to_8_binary(string("0x0"));
                }
                if(data_memory.count(t+1) > 0){
                    second_ = hex_to_8_binary(data_memory[t + 1]);
                }
                else{
                    second_ = hex_to_8_binary(string("0x0"));
                }
                if(data_memory.count(t+2) > 0){
                    third_ = hex_to_8_binary(data_memory[t + 2]);
                }
                else{
                    third_ =  hex_to_8_binary(string("0x0"));
                }
                if(data_memory.count(t+3) > 0){
                    last_ = hex_to_8_binary(data_memory[t + 3]);
                }
                else{
                    last_ = hex_to_8_binary(string("0x0"));
                }
                string entire = fir_ + second_ + third_ + last_;
                string hexx_ = bin_to_hex(entire);
                cout << hex << t <<": " << hexx_ << endl;
                t = t + 3;
            }
        }
    }
}
