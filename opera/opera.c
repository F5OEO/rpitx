#include <math.h>
#include <stdio.h>
#include <string.h>

char* itoa(unsigned long val, int n, char* buf, int radix)
{
  char c[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i = 0;
  while (n-i) {
    unsigned long d = (unsigned long)pow(radix, n-i-1);
    buf[i++] = c[val/d];
    val = val%d;
  }
  buf[i] = '\0';
  return buf;
}

int crc16op(char* msg, int n)
{
   int i,j,crc=0; // crc-16 (0x8005 poly, flip in and out, 2 zero bytes in tail) of msg[0..n-1]
   for(i=0;i!=n+2;i++){
    
     for(j=0;j!=8;j++)
       crc = crc&1 ? (crc>>1) ^ 0xa001 : crc>>1;
   }
   // replace, swap and store crc in msg[32..48]
   crc = crc&0xff00 ? ( crc&0x00ff ? crc : 0x001b|(crc&0xff00) ) : 0x2b00|(crc&0x00ff);
   crc = ((crc&0x00ff)<<8); // amp="amp" crc="crc" xff00="xff00">>8);
   return crc;
}

int main(int argc, const char* argv[]){
   if(argc != 2){
     printf("usage: opera [callsign]\n");
     return 0;
   }
   const char* call = argv[1];
   char msg[239];
   int i,j;

   char c[]="      "; //align last prefix digit at c[2] and fill gaps with blanks
   int aligned = isdigit(call[2]);
   strncpy(aligned ? c : &c[1], call, aligned ? 6 : 5); 
   //strupr(c); To implement : for now warning, should be in CAPITAL

   unsigned long n1=(c[0]>='0'&&c[0]<='9'?c[0]-'0'+27:c[0]==' '?0:c[0]-'A'+1); // packing call e.g. " K1JT ", "AA1AA " into n1
   n1=36*n1+(c[1]>='0'&&c[1]<='9'?c[1]-'0'+26:c[1]-'A');
   n1=10*n1+c[2]-'0';
   n1=27*n1+(c[3]==' '?0:c[3]-'A'+1);
   n1=27*n1+(c[4]==' '?0:c[4]-'A'+1);
   n1=27*n1+(c[5]==' '?0:c[5]-'A'+1);
   itoa(n1, 28, &msg[4], 2); //msg[4..32] = binary representation n1 in ASCII
   itoa(crc16op(&msg[4], 28), 16, &msg[32], 2); //store bin-crc of msg[4..31] in msg[32..47]
   itoa(crc16op(&msg[4], 28+16) & 0x07, 3, &msg[48], 2); //store bin-crc of msg[4..47] in msg[48..51]
   msg[0]=msg[1]=msg[2]=msg[3]='0';  //unused bits msg[0-3]

   const char* prn_vec = "111000010101011111100110110100000001100100010101011";
   for(i=0; i!=51; i++) //scramble
     msg[i] = ((msg[i]-'0') ^ (prn_vec[i]-'0')) +'0';

   const char* wh[] = {"0000000", "1010101", "0110011", "1100110", "0001111", "1011010", "0111100", "1101001"};
   for(i=(51/3)-1; i>=0; i--){  // Walsh-Hadamard encoding to msg[0..118]
     char b = (msg[i*3+0]-'0')*4+(msg[i*3+1]-'0')*2+(msg[i*3+2]-'0')*1;
     for(j=0; j!=7;j++)
       msg[i*7+j] = wh[b][j];
   }

   for(i=0; i!=7; i++)  // interleave 7x17 to msg[121..240]
     for(j=0; j!=17; j++)
       msg[121+j+17*i] = msg[i+7*j];

   for(i=0; i!=119; i++){  // Manchester encoding to msg[2..120]
     msg[2*i+1+2] = msg[121+i];
     msg[2*i+0+2] = (msg[2*i+1+2] == '0') + '0';
   }
   msg[0] = msg[1] ='1'; // head
   msg[239] = '\0'; // tail

   printf("message=%s symbols=%s\n", c, msg);
}
