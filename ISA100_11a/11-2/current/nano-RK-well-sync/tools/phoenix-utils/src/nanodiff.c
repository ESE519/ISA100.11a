#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELETE		0
#define INSERT		1
#define SUBSTITUTE	2

#define SHORT_BYTE 	4
#define LONG_BYTE	3

#define MAX_SIZE	(64*1024)
#define MAX_PATCH_SIZE	(16*1024)

#define START_OF_GROUP 	1
#define IN_GROUP	2
#define	NO_GROUP	3

//#define LOG //uncomment to generate log.txt


unsigned short i,j,p_count;


unsigned short offset;
unsigned long total;

unsigned char nfile[MAX_SIZE];
unsigned char ofile[MAX_SIZE];
unsigned char pfile[MAX_SIZE];
unsigned char ptemp[MAX_SIZE];

struct patch_entry
{
  unsigned char cmd;
  unsigned short addr;
  unsigned char data;
  unsigned char grouped_bytes;

  #ifdef LOG
  unsigned char olddata;
  #endif
}
patchEntry[MAX_PATCH_SIZE];


struct header
{
  unsigned char crc_oldimage;
}patch_header;


#ifdef LOG
int grp_count=0;
int max_grp=0;
int grp=0;
inline void logger(struct patch_entry *plog, int sn);
#endif

FILE* newFile; // create a new file pointer
FILE* oldFile;
FILE* f1;
FILE* f2;

inline int min(int a, int b, int c);
inline void encode_patch(struct patch_entry *p);
inline void group_bytes(int op);

// Function to compute old image CRC
unsigned char computeCRC(unsigned short len);


int main(int argc, char *argv[])
{
  char *oldfile, *newfile, *patch;

  unsigned short m=0, n=0, op=0, old;

  int z;

  unsigned short cost;

  p_count = 0;

  char s1,s2, temp;

  unsigned short  **d;

  volatile char c;
  total=0;

  if (argc != 4)
  {
    printf("Invalid Number of Arguments\n");
    printf("Usage: nanodiff <oldfile> <newfile> <patchfile>\n");
    return 1;free(d);
  }
  else
  {
    oldfile = argv[1];
    newfile = argv[2];
    patch = argv[3];
  }

  if((newFile=fopen(newfile,"rb"))==NULL)
  { // open a file
    printf("Could not open <newfile>\n"); // print an error
    exit(1);
  }

  if((oldFile = fopen(oldfile,"rb"))==NULL)
  { // open a file
    printf("Could not open <oldfile>\n"); // print an error
    exit(1);
  }

  while(!feof(oldFile))
  {
    n += fread(&temp,sizeof(char),1,oldFile);
    ofile[n-1] = temp;
  }

  // Calculate CRC for old image and store
  patch_header.crc_oldimage = computeCRC(n);

  while(!feof(newFile))
  {
    m += fread(&temp,sizeof(char),1,newFile);
    nfile[m-1] = temp;
  }

  printf("\noldFile Size: %d Bytes\n", n);
  printf("newFile Size: %d Bytes\n", m);

  m++;
  n++;

  total+=(m*sizeof(unsigned short *));
  d = malloc(m * sizeof(unsigned short *));
  if(d == NULL)
  {
    printf("out of memory 1\n");
    exit(1);
  }

  for(i = 0; i < m; i++)
  {
    total+=(m*sizeof(unsigned short *));
    d[i] = malloc(n * sizeof(unsigned short));
    if(d[i] == NULL)
    {
      printf("out of memory 2\n");
      exit(1);
    }
  }

  m--;
  n--;

  //printf("memory allocated\n");
printf( "Total Memory Allocated %lu (%lu M)\n",total,total/1024/1024);
  printf("\nDiffing...Wait\nBytes Read: 000000");

  for(i=0; i <= m; i++)
    d[i][0] = i;

  for(j=1; j <= n; j++)
    d[0][j] = j;

  for(i = 1; i <= m; i++)
  {
    printf("\b\b\b\b\b\b%06d",i);

    for(j = 1; j <= n; j++)
    {
      s1 = nfile[i-1];
      s2 = ofile[j-1];

      if (s1 == s2)
        cost = 0;
      else
        cost = 1;

      //printf("%c = %c so %d\n",s1[i-1],s2[j-1],cost);
      d[i][j] = min(d[i-1][j]+1,d[i][j-1]+1,d[i-1][j-1] + cost);

      //d[i-1, j] + 1,     // deletion
      //d[i, j-1] + 1,     // insertion
      //d[i-1, j-1] + coprintf(f1,"%d.\tdelete at %d\n",op,j+1);st   // substitution
    }
  }


  printf(" 100%%");
  printf("\n\nDifference: %d Bytes\n", d[m][n]);
  printf("\nCreating Patch File...Wait\nBytes Left: 000000");
  i = m;
  j = n;

  if((f1=fopen(patch,"wb"))==NULL)
  { // open a file
    printf("Could not create patch file\n"); // print an error
    exit(1);
  }


  //backtrace
  while(i > 0 || j > 0)
  {
    printf("\b\b\b\b\b\b%06d",i);
    old = d[i][j];
    if(i == 0)
    {
      i = i;
      j = j-1;

      if (d[i][j] != old)
      {
        patchEntry[op].addr = j;
        patchEntry[op].cmd = DELETE;
        #ifdef LOG
        patchEntry[op].olddata = ofile[j];
        #endif
        op++;
      }
    }
    else if(j == 0)
    {
      i = i-1;
      j = j;
      if (d[i][j] != old)
      {
        patchEntry[op].addr = j;
        patchEntry[op].cmd = INSERT;
        patchEntry[op].data = nfile[i];
        op++;
      }
    }
    else
    {
      if (d[i-1][j-1] <= d[i-1][j] &&	d[i-1][j-1] <= d[i][j-1])
      {
        i = i-1;
        j = j-1;
        if (d[i][j] != old)
        {
          patchEntry[op].addr = j;
          patchEntry[op].cmd = SUBSTITUTE;
          patchEntry[op].data = nfile[i];
          #ifdef LOG
          patchEntry[op].olddata = ofile[j];
          #endif
          op++;
        }
      }
      else if (d[i-1][j] <= d[i-1][j-1] && d[i-1][j] <= d[i][j-1])
      {
        i = i-1;
        j = j;
        if (d[i][j] != old)
        {
          patchEntry[op].addr = j;
          patchEntry[op].cmd = INSERT;
          patchEntry[op].data = nfile[i];
          op++;
        }
      }
      else if (d[i][j-1] <= d[i-1][j] && d[i][j-1] <= d[i-1][j-1])
      {
        i = i;
        j = j-1;

        if (d[i][j] != old)
        {
          patchEntry[op].addr = j;
          patchEntry[op].cmd = DELETE;
          #ifdef LOG
          patchEntry[op].olddata = ofile[j];
          #endif
          op++;
        }
      }
    }
  }


  free(d);

  #ifdef LOG
  if((f2=fopen("log.txt","w"))==NULL)
  { // open a file
    printf("Could not create log file\n"); // print an error
    exit(1);
  }
  fprintf(f2,"S.No.\tAddr\tCmd\tNew\tOld\t\t+INS\t+DEL\tI-D\tdDat\n\n");
  #endif

  group_bytes(op);



  offset = 0;
  //write patchEntry in reverse
  for(z = op -1; z >= 0; z--)
  {
    encode_patch(&patchEntry[z]);
    #ifdef LOG
    logger(&patchEntry[z],(op-z));
    #endif
  }

  #ifdef LOG
  fprintf(f2,"\n\nContinuous Byte Savings : %d Bytes\n",grp_count);
  fprintf(f2,"\n\nMax Grouped Byte : %d Bytes\n",max_grp);
  fclose(f2);
  #endif


  printf(" 100%%\n");
  printf("\nPatch File size: %d Bytes\n", p_count + sizeof(patch_header));
  printf("\nOld File CheckSUM: 0x%x\n", patch_header.crc_oldimage);

  // Write patch header with checksum
  fwrite(&patch_header.crc_oldimage,sizeof(patch_header),1,f1);
  // Write patch
  fwrite(pfile,sizeof(char),p_count,f1);

  fclose(newFile);
  fclose(oldFile);
  fclose(f1);

  printf("Patch file created: %s\n\n", patch);

  #ifdef LOG
  printf("Log Created: log.txt\n\n");
  #endif
  //printf("memory freed\n");

  return 0;
}


//find miniimum of three values
inline int min(int a, int b, int c)
{
  if (a <= b && a <= c)
    return(a);
  if (b <= c && b <= a)
    return(b);
  if (c <= a && c <= b)
    return(c);
}


//encode and write to the patch file
inline void encode_patch(struct patch_entry *p)
{
  int addr_size;
  int encode_mode = 0;
  unsigned char control_byte;
  static int group_mem=0;

  offset = p->addr - offset;

  if(p->grouped_bytes != 0)
  {
          encode_mode = START_OF_GROUP;
  }


  if(group_mem == 0)
  {
    //can fit in 6 bits?
    if(offset <= 63)
    {
      addr_size = SHORT_BYTE;
    }
    else
    {
      addr_size = LONG_BYTE;
    }

    switch(addr_size)
    {
      case SHORT_BYTE :
        if(encode_mode == START_OF_GROUP)
        {
          group_mem = p->grouped_bytes - 1;
          encode_mode = 0;
          control_byte = (unsigned char)(LONG_BYTE << 6) |
                          (unsigned char)(LONG_BYTE << 4) |
                          (unsigned char)(p->cmd << 2);
          pfile[p_count] = control_byte;
          p_count++;

          pfile[p_count] = (unsigned char)(p->grouped_bytes);
          p_count++;

          pfile[p_count] = (unsigned char)offset;
          p_count++;
        }
        else
        {
          control_byte = (unsigned char)(p->cmd << 6) |
                          (unsigned char)offset;
          pfile[p_count] = control_byte;
          p_count++;
        }
        break;

      case LONG_BYTE :
        if(encode_mode == START_OF_GROUP)
        {
          group_mem = p->grouped_bytes - 1;
          encode_mode = 0;
          control_byte = (unsigned char)(LONG_BYTE << 6) |
                            (unsigned char)(LONG_BYTE << 4) |
                            (unsigned char)(LONG_BYTE << 2) |
                            (unsigned char)(p->cmd);
          pfile[p_count] = control_byte;
          p_count++;
          pfile[p_count] = (unsigned char)(p->grouped_bytes);
          p_count++;
        }
        else
        {
          control_byte = (unsigned char)(LONG_BYTE << 6) |
                          (unsigned char)(p->cmd << 4) ;
          pfile[p_count] = control_byte;
          p_count++;
        }
        //endian independent
        pfile[p_count] = (unsigned char)(offset >> 8);
        p_count++;

        pfile[p_count] = (unsigned char)offset;
        p_count++;
        break;
    }
  }
  else
  {
    group_mem--;
  }

  switch(p->cmd)
  {
    case DELETE:
      break;
    case SUBSTITUTE:
      pfile[p_count] = p->data;
      p_count++;
      break;
    case INSERT:
      pfile[p_count] = p->data;
      p_count++;
      break;
  }

  #ifdef LOG
  if(offset <= 1)
  {
    grp_count++;
    grp++;
  }
  else
  {
    if(grp > max_grp)
      max_grp = grp;
    grp = 0;
  }
  #endif

  offset = p->addr;//set offset to last absolute address
}


#ifdef LOG
inline void logger(struct patch_entry *plog, int sn)
{

  static int inserts=0;
  static int deletes=0;

  switch(plog->cmd)
  {
    case DELETE :
      deletes++;
      fprintf(f2,"%d\t%d\tDEL\t---\t%d\t\t%d\t%d\t%d\t---\t%d\n",
                      sn,plog->addr,plog->olddata,inserts,deletes,
                      inserts - deletes,plog->grouped_bytes);
      break;

    case SUBSTITUTE :
      fprintf(f2,"%d\t%d\tSUB\t%d\t%d\t\t%d\t%d\t%d\t%d\t%d\n",
                      sn,plog->addr,plog->data,plog->olddata,
                      inserts,deletes,inserts - deletes,plog->data - plog->olddata,
                      plog->grouped_bytes);
      break;

    case INSERT :
      inserts++;
      fprintf(f2,"%d\t%d\tINS\t%d\t---\t\t%d\t%d\t%d\t---\t%d\n",
                      sn,plog->addr,plog->data,inserts,deletes,
                      inserts - deletes,plog->grouped_bytes);
      break;
  } 
}
#endif



//fills in the grouped_bytes entry
inline void group_bytes(int op)
{
  int i;
  int offset=0;
  int in_group=0;
  int group_count=1;
  int group_count_pos;
  int last_cmd = -1;

  for(i = op -1; i >= 0; i--)
  {
    offset = patchEntry[i].addr - offset;

    //group contiguous operations
    if(((offset == 1 && patchEntry[i].cmd != INSERT) ||
        (offset == 0 && patchEntry[i].cmd == INSERT)) &&
        (patchEntry[i].addr > 1) && (group_count <= 255) &&
        (last_cmd == patchEntry[i].cmd))
    {
      if(in_group == 1)
      {
        patchEntry[i].grouped_bytes = 0;
      }
      else
      {
        in_group = 1;
        group_count_pos = i + 1;
      }
      group_count++;

      if(last_cmd != DELETE && group_count > 3)
      {
        patchEntry[group_count_pos].grouped_bytes = group_count;
      }
      else if(last_cmd == DELETE && group_count > 2)
      {
        patchEntry[group_count_pos].grouped_bytes = group_count;
      }
    }
    else
    {
      in_group = 0;
      group_count = 1;
      patchEntry[i].grouped_bytes = 0;
    }
    last_cmd = patchEntry[i].cmd;
    offset = patchEntry[i].addr;
  }
}

// Compute CheckSUM for old image

unsigned char computeCRC(unsigned short len)
{
  int   i;
  unsigned char data = 0;

  for ( i = 0; i < len; i++ )
  {
    data += ofile[i];
  }
  return data;
}
