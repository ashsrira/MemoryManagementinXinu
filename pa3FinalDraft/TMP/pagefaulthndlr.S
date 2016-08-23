/* pagefaulthndlr.S -- the assembly file is used to store the error code and call the required service routine pfaulthndlr.c after which it must restore all the registers and the flags */

                .text
ec:             .long 0
                .globl pagefaulthndlr,ec

pagefaulthndlr:

        popl ec
        pushfl
        cli
        pushal
        call pfault
        popal
        popfl
        iret


