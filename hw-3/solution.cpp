#include<bits/stdc++.h>


int readInt(std::vector<unsigned char>& buffer, int& index) {
    int res = buffer[index] * (1 << 24) + buffer[index + 1] * (1 << 16) + buffer[index + 2] * (1 << 8)  + buffer[index + 3];
    index += 4;
    return res;
}

std::string readString(std::vector<unsigned char>& buffer, int& index) {
    int i = readInt(buffer, index);
    return "";
}

char readByte(std::vector<unsigned char>& buffer, int& index) {
    return buffer[index++];
}

int main(int argc, char* argv[]) {
    FILE* log = fopen("log", "w");

    std::ifstream input( argv[1], std::ios::binary );
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

    char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
    char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
    char *lds [] = {"LD", "LDA", "ST"};
    
    bool begin = 0;
    bool stop = 0;
    std::map<std::string, int> cnt;


    for (int index = 1; index < buffer.size() && !stop; ) {
        char x = buffer[index++];
        char h = (x & 0xF0) >> 4, l = x & 0x0F;
        if (h == 5 && l == 2) begin = 1;

        fprintf(log, "%d %d\n", h, l);
        if (!begin) continue;
        
        switch (h) {
        case 15:
            fprintf(log, "STOP!");
            stop = 1;
            break;
        
        case 0:
            fprintf(log, "BINOP\t%s", ops[l-1]);
        break;
        
        case 1:
            switch (l) {
            case  0:
                fprintf(log, "CONST\t%d", readInt(buffer, index));
                break;
                
            case  1:
                fprintf(log, "STRING\t%s", readString(buffer, index));
                break;
                
            case  2:
                fprintf(log, "SEXP\t%s ", readString(buffer, index));
                fprintf(log, "%d", readInt(buffer, index));
                cnt["SEXP"]++;
                break;
                
            case  3:
                fprintf(log, "STI");
                break;
                
            case  4:
                fprintf(log, "STA");
                break;
                
            case  5:
                fprintf(log, "JMP\t0x%.8x", readInt(buffer, index));
                cnt["JMP"]++;
                break;
                
            case  6:
                fprintf(log, "END");
                break;
                
            case  7:
                fprintf(log, "RET");
                break;
                
            case  8:
                fprintf(log, "DROP");
                break;
                
            case  9:
                fprintf(log, "DUP");
                break;
                
            case 10:
                fprintf(log, "SWAP");
                break;

            case 11:
                fprintf(log, "ELEM");
                break;
                
            default:
                fprintf(log, "FAIL");
            }
        break;
        
        case 2:
        case 3:
        case 4:
            fprintf(log, "%s\t", lds[h-2]);
            cnt[lds[h-2]]++;
            switch (l) {
            case 0: fprintf(log, "G(%d)", readInt(buffer, index)); break;
            case 1: fprintf(log, "L(%d)", readInt(buffer, index)); break;
            case 2: fprintf(log, "A(%d)", readInt(buffer, index)); break;
            case 3: fprintf(log, "C(%d)", readInt(buffer, index)); break;
            default: fprintf(log, "FAIL");
            }
        break;
        
        case 5:
        switch (l) {
            case  0:
                fprintf(log, "CJMPz\t0x%.8x", readInt(buffer, index));
                cnt["CJMPz"]++;
            
                break;
                
            case  1:
                fprintf(log, "CJMPnz\t0x%.8x", readInt(buffer, index));
                cnt["CJMPnz"]++;
            
                break;
                
            case  2:
                fprintf(log, "BEGIN\t%d ", readInt(buffer, index));
                fprintf(log, "%d", readInt(buffer, index));
                cnt["BEGIN"]++;
                break;
                
            case  3:
                fprintf(log, "CBEGIN\t%d ", readInt(buffer, index));
                fprintf(log, "%d", readInt(buffer, index));
                cnt["CBEGIN"]++;
                
                break;
                
            case  4:
                fprintf(log, "CLOSURE\t0x%.8x", readInt(buffer, index));
                cnt["CLOSURE"]++;
                
                {int n = readInt(buffer, index);
                for (int i = 0; i<n; i++) {
                switch (readByte(buffer, index)) {
                case 0: fprintf(log, "G(%d)", readInt(buffer, index)); break;
                case 1: fprintf(log, "L(%d)", readInt(buffer, index)); break;
                case 2: fprintf(log, "A(%d)", readInt(buffer, index)); break;
                case 3: fprintf(log, "C(%d)", readInt(buffer, index)); break;
                default: fprintf(log, "FAIL");
                }
                }
                };
                break;
                
            case  5:
                fprintf(log, "CALLC\t%d", readInt(buffer, index));
                cnt["CALLC"]++;
                
                break;
                
            case  6:
                fprintf(log, "CALL\t0x%.8x ", readInt(buffer, index));
                cnt["CALL"]++;
                
                fprintf(log, "%d", readInt(buffer, index));
                break;
                
            case  7:
                fprintf(log, "TAG\t%s ", readString(buffer, index));
                fprintf(log, "%d", readInt(buffer, index));
                break;
                
            case  8:
                fprintf(log, "ARRAY\t%d", readInt(buffer, index));
                break;
                
            case  9:
                fprintf(log, "FAIL\t%d", readInt(buffer, index));
                fprintf(log, "%d", readInt(buffer, index));
                break;
                
            case 10:
                fprintf(log, "LINE\t%d", readInt(buffer, index));
                break;

            default:
                fprintf(log, "FAIL");
        }
        break;
        
        case 6:
            fprintf(log, "PATT\t%s", pats[l]);
        break;

        case 7: {
            switch (l) {
            case 0:
                fprintf(log, "CALL\tLread");
                break;
                
            case 1:
                fprintf(log, "CALL\tLwrite");
                break;

            case 2:
                fprintf(log, "CALL\tLlength");
                break;

            case 3:
                fprintf(log, "CALL\tLstring");
                break;

            case 4:
                fprintf(log, "CALL\tBarray\t%d", readInt(buffer, index));
                break;

            default:
                fprintf(log, "FAIL");
            }
        }
        break;
        
        default:
            fprintf(log, "FAIL");
        }

        fprintf(log, "\n");
    }
    fclose(log);


    std::vector<std::pair<std::string, int>> values(cnt.begin(), cnt.end());
    std::sort(values.begin(), values.end(), [](auto& x, auto& y){ return x.second > y.second; });
    for (auto& code: values) {
        std::cout << code.first << ' ' << code.second << std::endl;
    }
}