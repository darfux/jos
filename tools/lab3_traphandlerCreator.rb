#env:ruby 2.0.0p247 (2013-06-27 revision 41674) [i686-linux]
#by fux
#for lab3's code in trapentry.S

def createTrapHandler(traps, traperror)
	table = Hash.new(false)

	traperror.each_line do |line|
		attrs = line.split(' ')
		intNum = attrs[-2]
		hasCode = attrs[-1]=="Yes"
		table[intNum.to_sym] = true if hasCode
	end

	traps.each_line do |trap|
		tmp = trap.split(' ')
		comment = tmp[3...tmp.size].join(' ') #not to_s
		intNum = tmp[2]
		type = tmp[1]
		func = type[1...type.length].downcase
		if table[intNum.to_sym]
			print "TRAPHANDLER(#{func}, #{type});"
		else
			print "TRAPHANDLER_NOEC(#{func}, #{type});"
		end
		puts "\t"+comment
	end
	puts
	for i in 20..47
		puts "TRAPHANDLER_NOEC(_#{i}, #{i});"
	end
	puts
	puts "TRAPHANDLER_NOEC(_syscall, T_SYSCALL);\nTRAPHANDLER_NOEC(_default, T_DEFAULT);"
end

def createSetGATE(traps)
	declare = ""
	setgate = ""
	traps.each_line do |trap|
		tmp = trap.split(' ')
		type = tmp[1]
		func = type[1...type.length].downcase
		declare += "void #{func}();\n"
		case type
			when "T_NMI"
				setgate += "SETGATE(idt[#{type}], 0, GD_KT, #{func}, 0);\n"
			when "T_BRKPT"
				setgate += "SETGATE(idt[#{type}], 1, GD_KT, #{func}, 3);\n"
			else
				setgate += "SETGATE(idt[#{type}], 1, GD_KT, #{func}, 0);\n"
		end
	end
	for i in 20..47
		declare += "void _#{i}();\n"
		setgate += "SETGATE(idt[#{i}], 1, GD_KT, _#{i}, 3);\n"
	end
	declare += "void _syscall();\nvoid _default();\n"
	setgate += "SETGATE(idt[T_SYSCALL], 1, GD_KT, _syscall, 3);\nSETGATE(idt[T_DEFAULT], 1, GD_KT, _default, 0);\n"

	print declare
	puts
	print setgate
end

#get from kern/trap.h
traps =<<tt
#define T_DIVIDE     0		// divide error
#define T_DEBUG      1		// debug exception
#define T_NMI        2		// non-maskable interrupt
#define T_BRKPT      3		// breakpoint
#define T_OFLOW      4		// overflow
#define T_BOUND      5		// bounds check
#define T_ILLOP      6		// illegal opcode
#define T_DEVICE     7		// device not available 
#define T_DBLFLT     8		// double fault
#define T_TSS       10		// invalid task switch segment
#define T_SEGNP     11		// segment not present
#define T_STACK     12		// stack exception
#define T_GPFLT     13		// genernal protection fault
#define T_PGFLT     14		// page fault
#define T_FPERR     16		// floating point error
#define T_ALIGN     17		// aligment check
#define T_MCHK      18		// machine check
#define T_SIMDERR   19		// SIMD floating point error
tt

#get from Error Code Summary @ http://pdos.csail.mit.edu/6.828/2006/readings/i386/s09_10.htm
traperror =<<te
Divide error                       0            No
Debug exceptions                   1            No
Breakpoint                         3            No
Overflow                           4            No
Bounds check                       5            No
Invalid opcode                     6            No
Coprocessor not available          7            No
System error                       8            Yes
Coprocessor Segment Overrun        9            No
Invalid TSS                       10            Yes
Segment not present               11            Yes
Stack exception                   12            Yes
General protection fault          13            Yes
Page fault                        14            Yes
Coprocessor error                 16            No
Two-byte SW interrupt             0-255         No
te

puts '//======TrapHandlers========'
createTrapHandler traps,traperror
puts
puts '//======SetGATE========'
createSetGATE traps
