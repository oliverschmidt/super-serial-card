.section .time_critical.firmware
.global firmware
.type firmware, %object
.balign 4
firmware:
.incbin "../341-0065-A.bin"
