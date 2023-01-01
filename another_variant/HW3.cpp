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


std::map<char, int> analyze_bytecodes(bytefile *bf)
{
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   throw  std::runtime_error("ERROR: invalid opcode")

  std::map<char, int> bytecode2count;
  
  char *ip     = bf->code_ptr;
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
        bytecode2count[x] += 1;
        INT;
        break;
        
      case  1:
        bytecode2count[x] += 1;
        STRING;
        break;
          
      case  2:
        bytecode2count[x] += 1;
        STRING;
        INT;
        break;
        
      case  3:
        break;
        
      case  4:
        break;
        
      case  5:
        bytecode2count[x] += 1;
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
        bytecode2count[x] += 1;
        INT;
        break;
      case 1:
        bytecode2count[x] += 1;
        INT;
        break;
      case 2: 
        bytecode2count[x] += 1;
        INT;
        break;
      case 3:
        bytecode2count[x] += 1;
        INT;
        break;
      default: FAIL;
      }
      break;
      
    case 5:
      switch (l) {
      case  0:
        bytecode2count[x] += 1;
        INT;
        break;
        
      case  1:
        bytecode2count[x] += 1;
        INT;
        break;
        
      case  2:
        bytecode2count[x] += 1;
        INT;
        INT;
        break;
        
      case  3:
        bytecode2count[x] += 1;
        INT;
        INT;
        break;
        
      case  4:
        // fprintf (f, "CLOSURE\t0x%.8x", INT);
        bytecode2count[x] += 1;
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
        bytecode2count[x] += 1;
        INT;
        break;
        
      case  6:
        // fprintf (f, "CALL\t0x%.8x ", INT);
        // fprintf (f, "%d", INT);
        bytecode2count[x] += 1;
        INT;
        INT;
        break;
        
      case  7:
        // fprintf (f, "TAG\t%s ", STRING);
        // fprintf (f, "%d", INT);
        bytecode2count[x] += 1;;
        INT;
        INT;
        break;
        
      case  8:
        // fprintf (f, "ARRAY\t%d", INT);
        bytecode2count[x] += 1;
        INT;
        break;
        
      case  9:
        // fprintf (f, "FAIL\t%d", INT);
        // fprintf (f, "%d", INT);
        bytecode2count[x] += 1;
        INT;
        INT;
        break;
        
      case 10:
        // fprintf (f, "LINE\t%d", INT);
        bytecode2count[x] += 1;
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
        bytecode2count[x] += 1;
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
  }
  while (1);
 stop: return bytecode2count;

}

void print_results(std::map<char, int>& m)
{
    std::vector<std::pair<int, char>> v;
    for (auto it = m.begin(); it != m.end(); ++it)
        v.push_back({it->second, it->first});

    std::sort(v.begin(), v.end());

    for (int i = 0; i < v.size(); ++i)
    {
        std::cout << std::hex << (int)v[i].second << " ";
        std::cout << std::dec << v[i].first << "\n";
    }
}


int main (int argc, char* argv[]) {
  bytefile *f = read_file (argv[1]);
  auto m = analyze_bytecodes(f);
  print_results(m);
  return 0;
}
