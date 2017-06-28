import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.PriorityQueue;

public class WordCount {

  private static class WC implements Comparable<WC> {
    final String word;
    final int count;
    WC(String word, int count) {
      this.word = word; this.count = count;
    }
    public int compareTo(WC other) {
      int n = count - other.count;
      return (n != 0) ? n : word.compareTo(other.word);
    }
  }

  private static class Queue {
    private final int _capacity;
    private final PriorityQueue<WC> _q;
    Queue(int maxCapacity) {
      _capacity = maxCapacity;
      _q = new PriorityQueue<WC>(maxCapacity);
    }

    void add(WC wc) {
      if (_q.size() < _capacity) {
        _q.add(wc);
      }
      else if (wc.compareTo(_q.peek()) > 0) {
        _q.remove();
        _q.add(wc);
      }
    }

    WC[] elements() {
      WC[] elements = _q.toArray(new WC[_q.size()]);
      Arrays.sort(elements);
      return elements;
    }
  }

  private static class WordCounts {
    private Map<String, Integer> _counts;

    WordCounts() { _counts = new HashMap<>(); }
    int get(String word) {
      Integer count = _counts.get(word);
      return (count == null) ? 0 : count;
    }
    void add(String word) {
      int count = get(word);
      _counts.put(word, count + 1);
    }

    void outMaxCounts(int n) {
      Queue q = new Queue(n);
      for (Map.Entry<String, Integer> entry : _counts.entrySet()) {
        WC wc = new WC(entry.getKey(), entry.getValue());
        q.add(wc);
      }
      WC[] elements = q.elements();
      for (int i = elements.length; i > 0; i--) {
        WC wc = elements[i - 1];
        System.out.format("%s %d\n", wc.word, wc.count);
      }
    }

  }

  private static class WordReader {
    private final StringBuilder _word;
    private final String _fileName;

    WordReader(String fileName) {
      _word = new StringBuilder();
      _fileName = fileName;
    }


    private boolean isWordChar(int c) {
      return Character.isLetterOrDigit(c) || c == '\'';
    }

    private void addWords(WordCounts counts, WordCounts stopWords) {
      try (BufferedReader in = new BufferedReader(new FileReader(_fileName))) {
        int c = 0;
        while (true) {
          while (c >= 0 && !isWordChar(c)) { c = in.read(); }
          if (c < 0) {
            return;
          }
          else {
            _word.setLength(0);
            do {
              _word.append((char)c);
              c = in.read();
            } while (isWordChar(c));
            String word = _word.toString().toLowerCase();
            if (stopWords.get(word) == 0) {
              counts.add(word);
            }
          }
        }
      }
      catch (IOException e) {
        throw new RuntimeException(e);
      }
    }

  } //class WordReader

  private static void usage() {
    System.err.format("usage: %s N STOP_FILE DATA_FILE...\n",
                      WordReader.class.getName());
    System.exit(1);
  }

  public static void main(String[] args) {
    if (args.length < 3) usage();
    int n = Integer.parseInt(args[0]);
    if (n < 0) usage();
    WordCounts stopWords = new WordCounts();
    (new WordReader(args[1])).addWords(stopWords, stopWords);
    WordCounts counts = new WordCounts();
    for (int i = 2; i < args.length; i++) {
      (new WordReader(args[i])).addWords(counts, stopWords);
    }
    counts.outMaxCounts(n);
  }

}
