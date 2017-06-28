#!/usr/bin/env ruby

#Writes each specified file N times to stdout.  If invoked with option -n
#translates newlines to spaces.

def process_file(filename, doNlSpace)
  File.open(filename, "r") do |f|
    f.each_char do |c|
      c = ' ' if c == "\n" && doNlSpace
      print c
    end
  end
end

def usage(prog)
  abort "usage: #{prog} -n N FILE...\n"
end

def go(prog, argv)
  usage(prog) if argv.size < 2;
  nlToSpace = argv[0] == '-n';
  argv.shift if nlToSpace
  usage(prog) if argv[0] !~ /^\d+$/
  n = argv.shift.to_i
  usage(prog) if argv.size == 0
  argv.each do |f| 
    (1..n).each { process_file(f, nlToSpace) }
  end
end

go($0, ARGV)
