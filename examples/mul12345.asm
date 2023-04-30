
main:
        addi    sp,sp,-32
        sw      s0,28(sp)
        addi    s0,sp,32
first:
        li      a5,1
        sw      a5,-20(s0)
second:
        # lw      a5,-20(s0)
        slli    a5,a5,1
        sw      a5,-20(s0)
third:
        # lw      a4,-20(s0)
        mv      a5,a4
        slli    a5,a5,1
        add     a5,a5,a4
        sw      a5,-20(s0)
fourth:
        # lw      a5,-20(s0)
        slli    a5,a5,2
        sw      a5,-20(s0)
fifth:
        # lw      a4,-20(s0)
        mv      a5,a4
        slli    a5,a5,2
        add     a5,a5,a4
        sw      a5,-20(s0)
done:
        li      a5,0
        mv      a0,a5
        # lw      s0,28(sp)
        addi    sp,sp,32
        jr      ra
