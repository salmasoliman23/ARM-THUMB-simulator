#include <stdio.h>
#include <stdlib.h>


int simulate (unsigned short);

unsigned char Mem[1024];    //array to simulate memory with 1 k addresses
unsigned int Regs[16];      //array to simulate the 16 register value
unsigned int PSR[32];       //status register as an array of flags
unsigned int SPSR[32];      //saved program status register
#define SIZE 32
#define SP  Regs[13]        // stack pointer
#define PC  Regs[15]
#define LR  Regs[14]
unsigned int stack[0x7FE] = { 0 };

int main()
{
  SP = 0x000007FE;
  FILE *fp;
  unsigned short inst_word;

  fp = fopen ("test2.bin", "rb");

  if (NULL == fp)
    {
      printf ("Cannot open the file\n");
      exit (0);
    }
    PC = 0x10;
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);
    fread (&inst_word, 2, 1, fp);


  while (fread (&inst_word, 2, 1, fp))
    {
      int PC_prev = PC;

      printf ("%08x\t%04x\t", PC, inst_word);
      simulate (inst_word);
      PC += 2;

      if(PC != PC_prev+2){
        fseek(fp, PC+2, SEEK_SET);
      }
    }

  fclose (fp);
  return 0;
}


int
simulate (unsigned short instr) // instr is 16 bit
{
  unsigned char fmt, op, offset5, rd, rs, ro, rb, offset3, rn, offset8, set,
    sign, b, l;

  fmt = (instr) >> 13;      // bring the first three bits from the left
  if (instr == 57005)
    exit(0);
    else
{
      switch (fmt)
    {
    case 0:     // format 1/2
      op = (instr >> 11) & 3;
      rd = instr & 7;   // last 3 bits from right
      rs = (instr >> 3) & 7;    //the next 3 bits from right
      offset5 = (instr >> 6) & 0x1F;    //next 5 bits from right
      if (op != 3)
        {           // format 1

          switch (op)
        {
        case 0:
          printf ("lsl\tr%d, r%d, #%d", rd, rs, offset5);
          Regs[rd] = Regs[rs] >> offset5;
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
          break;
        case 1:
          printf ("lsr\tr%d, r%d, #%d", rd, rs, offset5);
          Regs[rd] = Regs[rs] << offset5;
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
          break;
        case 2:
          printf ("asr\tr%d, r%d, #%d", rd, rs, offset5);
          Regs[rd] = Regs[rs] << offset5;
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
          break;

        }
        }
      else
        {           //add/sub/
          offset3 = rn = offset5 & 0x07;    //first 3 bits of offset5  from the right
          if ((offset5 & 0x08) == 0)
        {       //bit4 of offset5 =0 8=1000 add
          printf ("add\tr%d, r%d, ", rd, rs);
          if ((offset5 & 0x10) == 0)
            {       //bit 5 of offset5, if equal zero then its a register, else it is immediate
              Regs[rd] = Regs[rs] + Regs[rn];   // change in register value inside the array
              printf ("r%d\t// R%d=%d\t=\t%d\t+\t%d\n", rn, rd,
                  Regs[rd], Regs[rs], Regs[rn]);
            }
          else
            {
              printf ("#%d", offset3);
              Regs[rd] = Regs[rs] + offset3;
              printf ("\t//R%d=%d\t=\t%d\t+\t%d\n", rd, Regs[rd],
                  Regs[rs], offset3);
            }
        }
          else
        {
          printf ("sub\tr%d, r%d, ", rd, rs);
          if ((offset5 & 0x10) == 0)
            {
              printf ("r%d", rn);
              Regs[rd] = Regs[rs] - Regs[rn];
              printf ("\t//R%d=%d\t=\t%d\t-\t%d\n", rd, Regs[rd],
                  Regs[rs], Regs[rn]);
            }
          else
            {
              printf ("#%d", offset3);
              Regs[rd] = Regs[rs] - offset3;
              printf ("\t//R%d=%d\t=\t%d\t+\t%d\n", rd, Regs[rd],
                  Regs[rs], offset3);
            }
        }


        }
      break;
    case 1:
      op = (instr >> 11) & 3;
      rd = (instr >> 8) & 7;
      offset8 = instr & 0xFF;
      switch (op)
        {
        case 0:
          printf ("MOV\tr%d, #%d", rd, offset8);
          Regs[rd] = offset8;   // set the contents of register with offset value
          printf ("\t//R%d = %d\t=\t%d\t\n", rd, Regs[rd], offset8);
          break;
        case 1:
          printf ("CMP\tr%d, #%d", rd, offset8);
          //return compare result in PSR
          if (Regs[rd] >= offset8)
        PSR[29] = 1;
          printf ("\t//%d(R%d)\t>=\t%d\t?\t\tis%d\n", Regs[rd], rd,
              offset8, PSR[29]);
          break;
        case 2:
          printf ("ADD\tr%d, #%d", rd, offset8);
          Regs[rd] = Regs[rd] + offset8;    // add to the register value the value of offset
          printf ("\t//R%d= %d\t\n", rd, Regs[rd]);
          break;
        case 3:
          printf ("SUB\tr%d, #%d", rd, offset8);
          Regs[rd] = Regs[rd] - offset8;
          printf ("\t//R%d = %d\t\n", rd, Regs[rd]);
          break;
        }
      break;
    case 2:
      set = ((instr >> 11) & 0x3);
      if (set == 0)
        {
          if (((instr >> 10) & 0x1) == 0)
        {
          //  ALU operations      (format 4)
          op = (instr >> 6) & 0xF;
          rd = instr & 7;   // last 3 bits from right
          rs = (instr >> 3) & 7;    //the next 3 bits from right
          switch (op)
            {
            case 0:
              printf ("AND\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] & Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 1:
              printf ("EOR\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] | Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 2:
              printf ("LSL\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] << Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 3:
              printf ("LSR\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] >> Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 4:
              printf ("ASR\t r%d, r%d", rd, rs);
              sign = (Regs[rd] >> 31) & 0x1;    // sign bit, first bit from the left
              Regs[rd] = Regs[rd] << Regs[rs];
              Regs[rd] = (((Regs[rd] << 1) >> 1) | (sign << 31));
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 5:
              printf ("ADC\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] + Regs[rs] + PSR[29];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 6:
              printf ("SBC\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] - Regs[rs] + !PSR[29];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 7:
              printf ("ROR\t r%d, r%d", rd, rs);
              for (int i = 0; i < Regs[rs]; i++)
            {   // loop by the value saved in rs register
              Regs[rd] = ((Regs[rd] >> 31) & 0x1) | (Regs[rd] << 1);    // rotate the last bit from right to be the first bit from left
            }
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 8:
              printf ("TST\tr%d, r%d", rd, rs);
              if ((Regs[rd] & Regs[rs]) == 0)
            {
              PSR[30] = 1;
            }
              else
            PSR[30] = 0;
              printf ("\t//%d\t&%d\t =%d\n", Regs[rd], Regs[rs],
                  PSR[30]);
              break;
            case 9:
              printf ("NEG\tr%d, r%d", rd, rs);
              Regs[rd] = Regs[rs] * -1;
              printf ("\t//R%d = %d\t\n", rd, Regs[rd]);
              break;
            case 10:
              printf ("CMP\tr%d, r%d", rd, rs);
              if ((Regs[rd] - Regs[rs]) == 0)
            PSR[30] = 1;
              else if ((Regs[rd] - Regs[rs]) < 0)
            PSR[31] = 1;
              printf ("%d\t%d\t%d\t%d\n", Regs[rd], Regs[rs], PSR[30],
                  PSR[31]);
              break;
            case 11:
              printf ("CMN\tr%d, r%d", rd, rs);
              if ((Regs[rd] + Regs[rs]) == 0)
            PSR[30] = 1;
              else if ((Regs[rd] + Regs[rs]) < 0)
            PSR[31] = 1;
              printf ("\t//%d\t%d\t%d\t%d\n", Regs[rd], Regs[rs],
                  PSR[30], PSR[31]);
              break;
            case 12:
              printf ("ORR\tr%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] | Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 13:
              printf ("MUL\tr%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] * Regs[rs];
              printf ("\t//R%d = %d\n", rd, Regs[rd]);
              break;
            case 14:
              printf ("BIC\t r%d, r%d", rd, rs);
              Regs[rd] = Regs[rd] & (4294967295 - Regs[rs]);    //convert contents of rs reg into its one's complement (0xFFFFFFFF - number for the 32 bit value )
              printf ("\t// BIC R%d %d\t%d\n", rd, Regs[rd],
                  Regs[rs]);
              break;
            case 15:
              printf ("MVN\t r%d, r%d", rd, rs);
              Regs[rd] = (4294967295 - Regs[rs]);   //ones complement again
              printf ("\tR%d = %d\t\n", rd, Regs[rd]);
              break;
            }

        }
          else
        {
          //Hi register operations/branch exchange (instruction format 5, not listed in the project)
          printf ("NOT INCLUDED INS. \n");
        }
        }
      else if (set == 1)
        {
          //PC relative load!  (format 6)

          offset8 = instr & 0xff;
          rd = (instr >> 8) & 0x7;
          printf ("LDR\t r%d, [ PC, %d ]", rd, offset8);
          Regs[rd] = Mem[Regs[15] + offset8];   // add the immediate value to the content of the pc then load the content of memory at the resulted address into rd register
          printf ("\t//R%d = %d\n", rd, Regs[rd]);


        }
      else if (((instr >> 9) & 0x1) == 0)
        {
          // l/s with register offset   (format 7)

          rd = instr & 0x7;
          rb = (instr >> 3) & 0x7;  //rb base register
          ro = (instr >> 6) & 0x7;  // offset register
          b = (instr >> 10) & 0x1;
          l = (instr >> 11) & 0x1;
          if (b == 0 & l == 0)
        {
          printf ("STR\tr%d, [r%d, r%d]", rd, rb, ro);
          Mem[Regs[rb] + Regs[ro]] = Regs[rd];
          printf ("\t//Mem =%d\n", Regs[rd]);

        }
          else if (b == 1 & l == 0)
        {
          printf ("STRB\tr%d, [r%d, r%d]", rd, rb, ro);
          Mem[Regs[rb] + Regs[ro]] = Regs[rd] & 0xFF;   // to load first byte only
          printf ("\t//Mem=%d\n", Regs[rd]);
        }
          else if (b == 0 & l == 1)
        {
          printf ("LDR\tr%d, [r%d, r%d]", rd, rb, ro);
          Regs[rd] = Mem[Regs[rb] + Regs[ro]];
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
        }
          else
        {
          printf ("LDRB\tr%d, [r%d, r%d]", rd, rb, ro);
          Regs[rd] = Mem[Regs[rb] + Regs[ro]] & 0xFF;   // to load first byte only
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
        }

        }
      else
        {
          //Load/store sign-extended byte/halfword  ( NOT INCLUDED IN THE PROJECT) (format 8)
          printf ("NOT INCLUDED INS. \n");
        }
      break;
    case 3:     //(format 9)
      rd = instr & 0x7;
      rb = (instr >> 3) & 0x7;
      offset5 = (instr >> 6) & 0x1F;
      l = (instr >> 11) & 0x1;
      b = (instr >> 12) & 0x1;
      if (b == 0 & l == 0)
        {
          printf ("STR\tr%d, [r%d, #%d]", rd, rb, offset5);
          Mem[Regs[rs] + offset5] = Regs[rd];
          printf ("\t//Mem=%d\n", Regs[rd]);
        }
      else if (b == 1 & l == 0)
        {
          printf ("STRB\tr%d, [r%d, #%d]", rd, rb, offset5);
          Mem[Regs[rs] + offset5] = Regs[rd] & 0xFF;    // to load first byte only
          printf ("\t//Mem=%d\n", Regs[rd]);
        }
      else if (b == 0 & l == 1)
        {
          printf ("LDR\tr%d, [r%d, #%d]", rd, rb, offset5);
          Regs[rd] = Mem[Regs[rs] + offset5];
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
        }
      else
        {
          printf ("LDRB\tr%d, [r%d, #%d]", rd, rb, offset5);
          Regs[rd] = Mem[Regs[rs] + offset5] & 0xFF;    // to load first byte only
          printf ("\t//R%d = %d\n", rd, Regs[rd]);
        }
      break;
    case 4:
      // format 10,11, not included in the project
      printf ("NOT INCLUDED INS. \n");
      break;
    case 5:
      if (((instr >> 12) & 0x1) == 0)
        {
          printf ("NOT INCLUDED INS. \n");  //load address (not included in the project)(format 12)
        }
      else if (((instr >> 10) & 0x1) == 0)
        {           //(format 13)
          unsigned int sword7, s;
          sword7 = (instr & 0x7F) << 2;
          s = (instr >> 7) & 0x1;
          if (s == 0)
        {
          printf ("ADD\tSP , #%d", sword7);
          Regs[13] += sword7;   //increase the value of sp by immediate
          printf ("\t//SP=%d\n", SP);

        }
          else
        {
          printf ("ADD\tSP , #-%d", sword7);
          Regs[13] -= sword7;   // decrease the value of sp by immediate
          printf ("\t//SP=%d\n", SP);
        }


        }
      else
        {           //format-14
          unsigned int Rlist = instr & 0xff; //anded with 8-ones
          unsigned char R = (instr >> 8) & 0x1;
          unsigned char L = (instr >> 11) & 0x1;
          int start = Rlist & 0xF;
          int end = (Rlist >> 4) & 0xF;

          if (L == 0 && R == 0)
        {
          if (SP == 0)
            printf ("STACK IS FULL\n");
          else
            {
              printf ("PUSH{R%d, R%d}", start, end);
              for (int i = start; i <= end; i++)
            {
              stack[SP] = Regs[i];
              printf ("\t//R%d=%d Pushed", i, stack[SP]);
              SP = SP - 1;
              printf ("\t//SP = %d\n", SP);
            }
        }}
          else if (L == 0 && R == 1)
        {
          if (SP == 0)
            printf ("STACK IS FULL\n");
          else
            {
              printf ("PUSH{R%d, R%d, LR}", start, end);

              for (int i = start; i <= end; i++)
            {
              stack[SP] = Regs[i];
              printf ("\t//%d Pushed", stack[SP]);
              SP = SP - 1;
              printf ("\tSP =%d\n", SP);

            }
              stack[SP] = LR;
              printf ("%d Pushed \t the value of LR is %d ", stack[SP], LR);
              SP = SP - 1;
              printf ("\tSP = %d\n", SP);
        }}
          else if (L == 1 && R == 0)
        {
          if (SP == 0x000007FE)
            printf ("STACK is EMPTY\n");
          else
            {
              printf ("POP{ R%d, R%d}", start, end);
              for (int i = start; i <= end; i++)
            {
              Regs[i] = stack[SP];
              printf ("\t//%d Poped", Regs[i]);
              SP = SP + 1;
              printf ("\tSP = %d\n", SP);
            }
        }}
          if (L == 1 && R == 1)
        {
          if (SP == 0x000007FE)
            printf ("STACK is EMPTY\n");
          else
            {
              printf ("POP{R%d, R%d, PC}", start, end);
              PC = stack[SP];
              printf ("\t//PC = %d", PC);
              SP = SP + 1;
              printf (" \tSP = %d\n", SP);
              for (int i = start; i <= end; i++)
            {
              Regs[i] = stack[SP];
              printf ("\t%d Poped", Regs[i]);
              SP = SP + 1;
              printf ("\tSP = %d\n", SP);
            }}

        }
        }
      break;
    case 6:
        if ((instr >> 12) & 0x1 == 0)
          printf ("Not Included in the project \n");    //format 15
        else
        {         // format 16

                unsigned int cond = (instr >> 8) & 0xf;
                offset8 = (instr & 0xff) >> 1;
                switch (cond)
                  {
                  case 0:
                      printf ("BEQ\t%d\n", offset8);
                      if (PSR[30])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 1:
                      printf ("BNE\t%d", offset8);
                      if (!PSR[30])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 2:
                      printf ("BCS\t%d", offset8);
                      if (PSR[29])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 3:
                      printf ("BCC\t%d", offset8);
                      if (!PSR[29])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 4:
                      printf ("BMI\t%d", offset8);
                      if (PSR[31])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 5:
                      printf ("BPL\t%d", offset8);
                      if (!PSR[31])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 6:
                      printf ("BVS\t%d", offset8);
                      if (PSR[28])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 7:
                      printf ("BVC\t%d", offset8);
                      if (!PSR[28])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 8:
                      printf ("BHI\t%d", offset8);
                      if (PSR[29] && !PSR[30])
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 9:
                      printf ("BLS\t%d\n", offset8);
                      if (!PSR[29] || PSR[30])
                    {
                      PC = offset8-2;
                      printf ("%d\n", PC);
                    }
                    break;
                  case 10:
                      printf ("BGE\t%d", offset8);
                      if ((PSR[28] && PSR[31]) || (!PSR[28] && !PSR[31]))
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 11:
                      printf ("BLT\t%d", offset8);
                      if ((!PSR[28] && PSR[31]) || (PSR[28] && !PSR[31]))
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 12:
                      printf ("BGT\t%d", offset8);
                      if (PSR[30]
                      && ((PSR[28] && PSR[31]) || (!PSR[28] && !PSR[31])))
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 13:
                      printf ("BLE\t%d", offset8);
                      if (PSR[30] || (!PSR[28] && PSR[31]) || (PSR[28] && !PSR[31]))
                    {
                      PC = offset8-2;
                      printf ("\t//PC updated to %d\n", PC);
                    }
                    break;
                  case 14:
                    printf ("NOT INCLUDED\n");
                    break;
                  case 15:  //format 17
                    
                      int value8 = instr & 0xFF;

                      printf ("SWI\t%d\n", value8);
                      LR = PC;  // Move the address of the next instruction into LR
                      for (int i = 0; i <= 4; i++){
                      SPSR[i] = PSR[i]; //  move CPSR to SPSR
                    }
                      PC = 0x8; //load the SWI vector address (0x8) into the PC
                      PSR[0] = 0x00000008;  // update PSR with supervisor mode

                switch (value8){
                            case 1://printing the value in Regs[0]
                              printf ("%d\n", Regs[0]);
                              break;

                            case 2: // printing a char
                              printf ("%c\n", Regs[0]);
                              break;

                            case 3:{ //read string
                              int j, i;
                              int count = 0;
                              char string[Regs[0]];
                              // taking the string character by character
                              for (j = 0; j < Regs[0]; j++)
                                {
                                  scanf ("%s", &string[j]);  // string+j;
                                }
                              // searching for Regs[0] consequitive free spaces in memory!
                                i = 0;
                                j = 0;
                              while (i <= 1023 && count <= Regs[0])
                                {
                                  if (Mem[i] != NULL)
                                    count = 0;
                                  else
                                    count++;
                                  i++;
                                }
                              if (count < Regs[0])
                                {
                                  printf ("There is no enough memory\n");
                                  break;
                                }

                              int end = i - 1;
                              int start = Regs[0] - end;

                              i = start;
                              while (i <= end)
                                {
                                  Mem[i] = string[j];
                                  printf(" Memory of address %u has been updated to %c\n", i, Mem[i]);
                                  i++;
                                  j++;
                                }
                            }
                              break;

                            case 4: // reading a character
                              char character;
                              scanf ("%c", &character);
                              Regs[0] = character;
                              break;

                              // assuming the address of the string that's inside the code is in Regs[0]
                            case 5:{ /////print string
                              int q = Regs[0];
                              while (Mem[q] != 0)
                                {
                                  printf ("%c", Mem[q]);
                                  q++;
                                }
                            }
                                break;
                            case 6:// reading an integer
                              int integer;
                              scanf ("%d", &integer);
                              Regs[0] = integer;
                              break;
                            case 7:
                              printf ("PROGRAM TERMINATED!\n");
                              exit (0);
                              break;    // terminating the program!
                        }

                        PC = LR;
                      for (int i = 0; i <= 4; i++){
                      PSR[i] = SPSR[i]; //  move SPSR back to CPSR 
                    }
                  break;
              }
          }
              break;

    case 7:{
        if (!((instr >> 12) & 1))
          {         //format 18  if bit 12=0
        int offset11;
        if (((instr >> 10) & 0x1) == 0){   //if offset is positive
          offset11 = instr & 0x7ff;
          offset11 = offset11 << 1;
        }

        else
          offset11 = 2048 - ((instr & 0x7ff) << 1); //if offset is negative
        
        printf ("B\t%d", offset11);
        PC += offset11 - 2; // offset is the step from the current offset
        printf ("\t//PC updated to %d\n", PC);
          }
        else
          {         //format 19  if bit 12 =1
        int offset = (instr & 0x7ff);
        unsigned char H = (instr >> 11) & 0x1;

        if (H == 0)
          {
            printf ("BL\t %d", offset);
            LR = PC + (offset << 12);
            printf ("\t//LR updated to %d\n", LR);
          }
        else
          {
            printf ("BL\t %d", offset);
            int temp = LR + 4;  // next instruction to be excuted
            PC = LR + (offset << 1);
            printf ("\t//PC updated to %d", PC);
            LR = temp | 0x1;
            printf ("\t and LR updated to %d\n", LR);

          }
          }
      }
          break;

    default:
        printf ("UNKNOWN INSTR!\n");
      break;
      }
    }
    }