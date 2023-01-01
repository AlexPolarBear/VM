#include <iostream>
#include <exception>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <map>
#include <string>
#include <string.h>
#include <utility>
#include <algorithm>
#include <vector>

void *__start_custom_data;
void *__stop_custom_data;

/* The unpacked representation of bytecode file */
typedef struct {
  char *string_ptr;              /* A pointer to the beginning of the string table */
  int  *public_ptr;              /* A pointer to the beginning of publics table    */
  char *code_ptr;                /* A pointer to the bytecode itself               */
  int  *global_ptr;              /* A pointer to the global area                   */
  int   stringtab_size;          /* The size (in bytes) of the string table        */
  int   global_area_size;        /* The size (in words) of global area             */
  int   public_symbols_number;   /* The number of public symbols                   */
  char  buffer[0];               
} bytefile;

struct bytecode {
    char* begin;
    size_t length;

    bytecode(char* bytecode_begin, size_t bytecode_length) {
        begin = bytecode_begin;
        length = bytecode_length;
    }
};

bool operator==(const bytecode& lhs, const bytecode& rhs) {
    if (lhs.length != rhs.length)
        return false;

    for (int i = 0; i < lhs.length; ++i) {
        if (*(lhs.begin + i) != *(rhs.begin + i))
            return false;
    }
    return true;
} 




/* Gets a string from a string table by an index */
char* get_string (bytefile *f, int pos) {
  return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char* get_public_name (bytefile *f, int i) {
  return get_string (f, f->public_ptr[i*2]);
}

/* Gets an offset for a publie symbol */
int get_public_offset (bytefile *f, int i) {
  return f->public_ptr[i*2+1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile* read_file (char *fname) {
  FILE *f = fopen (fname, "rb");
  long size;
  bytefile *file;

  if (f == 0) {
    // failure ("%s\n", strerror (errno));
  }
  
  if (fseek (f, 0, SEEK_END) == -1) {
    // failure ("%s\n", strerror (errno));
  }

  file = (bytefile*) malloc (sizeof(int)*4 + (size = ftell (f)));

  if (file == 0) {
    // failure ("*** FAILURE: unable to allocate memory.\n");
  }
  
  rewind (f);

  if (size != fread (&file->stringtab_size, 1, size, f)) {
    //failure ("%s\n", strerror (errno));
  }
  
  fclose (f);
  
  file->string_ptr  = &file->buffer [file->public_symbols_number * 2 * sizeof(int)];
  file->public_ptr  = (int*) file->buffer;
  file->code_ptr    = &file->string_ptr [file->stringtab_size];
  file->global_ptr  = (int*) malloc (file->global_area_size * sizeof (int));
  
  return file;
}


std::vector<std::pair<bytecode, int>> analyze_bytecodes(bytefile *bf)
{
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   throw  std::runtime_error("ERROR: invalid opcode")

  std::vector<std::pair<bytecode, int>> bytecode2count;
  
  char *ip     = bf->code_ptr;
  char* old_ip = ip;
  do {
    char x = BYTE,
         h = (x & 0xF0) >> 4,
         l = x & 0x0F;

    switch (h) {
    case 15:
      goto stop;
      
    /* BINOP */
    case 0:
      break;
      
    case 1:
      switch (l) {
      case  0:
        INT;
        break;
        
      case  1:
        STRING;
        break;
          
      case  2:
        STRING;
        INT;
        break;
        
      case  3:
        break;
        
      case  4:
        break;
        
      case  5:
        INT;
        break;
        
      case  6:
        break;
        
      case  7:
        break;
        
      case  8:
        break;
        
      case  9:
        break;
        
      case 10:
        break;

      case 11:
        break;
        
      default:
        FAIL;
      }
      break;
      
    case 2:
    case 3:
    case 4:
      switch (l) {
      case 0: 
        INT;
        break;
      case 1:
        INT;
        break;
      case 2: 
        INT;
        break;
      case 3:
        INT;
        break;
      default: FAIL;
      }
      break;
      
    case 5:
      switch (l) {
      case  0:
        INT;
        break;
        
      case  1:
        INT;
        break;
        
      case  2:
        INT;
        INT;
        break;
        
      case  3:
        INT;
        INT;
        break;
        
      case  4:
        // fprintf (f, "CLOSURE\t0x%.8x", INT);
        INT;

        {int n = INT;
         for (int i = 0; i<n; i++) {
         switch (BYTE) {
           case 0: INT; break;
           case 1: INT; break;
           case 2: INT; break;
           case 3: INT; break;
           default: FAIL;
         }
         }
        };
        break;
          
      case  5:
        // fprintf (f, "CALLC\t%d", INT);
        INT;
        break;
        
      case  6:
        // fprintf (f, "CALL\t0x%.8x ", INT);
        // fprintf (f, "%d", INT);
        INT;
        INT;
        break;
        
      case  7:
        // fprintf (f, "TAG\t%s ", STRING);
        // fprintf (f, "%d", INT);
        INT;
        INT;
        break;
        
      case  8:
        // fprintf (f, "ARRAY\t%d", INT);
        INT;
        break;
        
      case  9:
        // fprintf (f, "FAIL\t%d", INT);
        // fprintf (f, "%d", INT);
        INT;
        INT;
        break;
        
      case 10:
        // fprintf (f, "LINE\t%d", INT);
        INT;
        break;

      default:
        FAIL;
      }
      break;
      
    case 6:
      // fprintf (f, "PATT\t%s", pats[l]);
      break;

    case 7: {
      switch (l) {
      case 0:
        // fprintf (f, "CALL\tLread");
        break;
        
      case 1:
        // fprintf (f, "CALL\tLwrite");
        break;

      case 2:
        // fprintf (f, "CALL\tLlength");
        break;

      case 3:
        // fprintf (f, "CALL\tLstring");
        break;

      case 4:
        // fprintf (f, "CALL\tBarray\t%d", INT);
        INT;
        break;

      default:
        FAIL;
      }
    }
    break;
      
    default:
      FAIL;
    }
    size_t length = ip - old_ip;
    bytecode b = bytecode(old_ip, length);
    bool added = false;
    for (int i = 0; i < bytecode2count.size(); ++i) {
        if (bytecode2count[i].first == b) {
            bytecode2count[i].second++;
            added = true;
            break;
        }
    }
    if (!added) {
        bytecode2count.push_back({b, 1});
    }
    old_ip = ip;
  }
  while (1);
 stop: return bytecode2count;

}

void print_bytecode(FILE *f,const bytecode& b, bytefile* bf) {
  
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   throw  std::runtime_error("ERROR: invalid opcode")
  
  char *ip     = b.begin;
  const char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
  const char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
  const char *lds [] = {"LD", "LDA", "ST"};
    char x = BYTE,
         h = (x & 0xF0) >> 4,
         l = x & 0x0F;
    
    switch (h) {
    case 15:
      goto stop;
      
    /* BINOP */
    case 0:
      fprintf (f, "BINOP\t%s", ops[l-1]);
      break;
      
    case 1:
      switch (l) {
      case  0:
        fprintf (f, "CONST\t%d", INT);
        break;
        
      case  1:
        fprintf (f, "STRING\t%s", STRING);
        break;
          
      case  2:
        fprintf (f, "SEXP\t%s ", STRING);
        fprintf (f, "%d", INT);
        break;
        
      case  3:
        fprintf (f, "STI");
        break;
        
      case  4:
        fprintf (f, "STA");
        break;
        
      case  5:
        fprintf (f, "JMP\t0x%.8x", INT);
        break;
        
      case  6:
        fprintf (f, "END");
        break;
        
      case  7:
        fprintf (f, "RET");
        break;
        
      case  8:
        fprintf (f, "DROP");
        break;
        
      case  9:
        fprintf (f, "DUP");
        break;
        
      case 10:
        fprintf (f, "SWAP");
        break;

      case 11:
        fprintf (f, "ELEM");
        break;
        
      default:
        FAIL;
      }
      break;
      
    case 2:
    case 3:
    case 4:
      fprintf (f, "%s\t", lds[h-2]);
      switch (l) {
      case 0: fprintf (f, "G(%d)", INT); break;
      case 1: fprintf (f, "L(%d)", INT); break;
      case 2: fprintf (f, "A(%d)", INT); break;
      case 3: fprintf (f, "C(%d)", INT); break;
      default: FAIL;
      }
      break;
      
    case 5:
      switch (l) {
      case  0:
        fprintf (f, "CJMPz\t0x%.8x", INT);
        break;
        
      case  1:
        fprintf (f, "CJMPnz\t0x%.8x", INT);
        break;
        
      case  2:
        fprintf (f, "BEGIN\t%d ", INT);
        fprintf (f, "%d", INT);
        break;
        
      case  3:
        fprintf (f, "CBEGIN\t%d ", INT);
        fprintf (f, "%d", INT);
        break;
        
      case  4:
        fprintf (f, "CLOSURE\t0x%.8x", INT);
        {int n = INT;
         for (int i = 0; i<n; i++) {
         switch (BYTE) {
           case 0: fprintf (f, "G(%d)", INT); break;
           case 1: fprintf (f, "L(%d)", INT); break;
           case 2: fprintf (f, "A(%d)", INT); break;
           case 3: fprintf (f, "C(%d)", INT); break;
           default: FAIL;
         }
         }
        };
        break;
          
      case  5:
        fprintf (f, "CALLC\t%d", INT);
        break;
        
      case  6:
        fprintf (f, "CALL\t0x%.8x ", INT);
        fprintf (f, "%d", INT);
        break;
        
      case  7:
        fprintf (f, "TAG\t%s ", STRING);
        fprintf (f, "%d", INT);
        break;
        
      case  8:
        fprintf (f, "ARRAY\t%d", INT);
        break;
        
      case  9:
        fprintf (f, "FAIL\t%d", INT);
        fprintf (f, "%d", INT);
        break;
        
      case 10:
        fprintf (f, "LINE\t%d", INT);
        break;

      default:
        FAIL;
      }
      break;
      
    case 6:
      fprintf (f, "PATT\t%s", pats[l]);
      break;

    case 7: {
      switch (l) {
      case 0:
        fprintf (f, "CALL\tLread");
        break;
        
      case 1:
        fprintf (f, "CALL\tLwrite");
        break;

      case 2:
        fprintf (f, "CALL\tLlength");
        break;

      case 3:
        fprintf (f, "CALL\tLstring");
        break;

      case 4:
        fprintf (f, "CALL\tBarray\t%d", INT);
        break;

      default:
        FAIL;
      }
    }
    break;
      
    default:
      FAIL;
    }

    fprintf (f, "\n");
    stop:;
}


void print_results(std::vector<std::pair<bytecode, int>>& v, bytefile* bf)
{
    std::sort(v.begin(), v.end(),
              [](const std::pair<bytecode, int>& lhs, const std::pair<bytecode, int>& rhs)
              {
                return lhs.second < rhs.second;
              });

    for (int i = 0; i < v.size(); ++i)
    {
        std::cout << v[i].second << "\t";
        print_bytecode(stdout, v[i].first, bf);
    }
}


int main (int argc, char* argv[]) {
  bytefile *f = read_file (argv[1]);
  auto m = analyze_bytecodes(f);
  print_results(m, f);
  return 0;
}
