#!/usr/bin/env ruby

#Needs algorithms library for Containers::MinHeap
require 'algorithms'

WC = Struct.new(:word, :count) do
  include Comparable
  def <=>(other) 
    n = count <=> other.count
    return (n != 0) ? n : (word <=> other.word)
  end
end

class MaxCounts
  def initialize(maxN, wordCounts)
    @maxN = maxN
    @q = Containers::MinHeap.new
    wordCounts.each { |wc| add(wc) }
  end

  def out_words
    #Containers::Heap seems to be missing each(); 
    #hence following conversion routine
    words = []
    while @q.size > 0
      words.push @q.min!
    end
    words.reverse.each { |wc| print "#{wc.word} #{wc.count}\n" }
  end

  private

  def add(wc)
    if (@q.size < @maxN)
      @q.push(wc)
    elsif (wc <=> @q.min) > 0
      @q.min!
      @q.push(wc)
    end
  end

end

class WordCounts

  include Enumerable

  def initialize
    @counts = Hash.new(0)
  end

  def [](word)
    @counts[word]
  end

  def []=(word, count)
    @counts[word] = count
  end

  def each
    @counts.each_pair { |word, count| yield WC.new(word, count); }
  end

end

class WordReader
  
  def initialize(filename)
    @filename = filename
  end

  def is_word_char(c)
    c =~ /^[[:alnum:]\']$/
  end

  def add_words(stops, counts)
    w = ''
    File.open(@filename, "r") do |f|
      f.each_char do |c|
        if is_word_char(c)
          w << c
        elsif w.size > 0
          w.downcase!
          counts[w] += 1 if stops[w] == 0
          w = ''
        end
      end
    end
    w.downcase!
    counts[w] += 1 if w.size > 0 && stops[w] == 0
  end

end

def go(argv)
  n = argv[0].to_i
  stops = WordCounts.new
  WordReader.new(argv[1]).add_words(stops, stops)
  counts = WordCounts.new
  (2...argv.size).each { |i| WordReader.new(argv[i]).add_words(stops, counts) }
  MaxCounts.new(n, counts).out_words    
end

abort "usage: $0 N STOP_WORDS_FILE DATA_FILE..." if 
    ARGV.size < 3 || ARGV[0].to_i < 0
go(ARGV)
