package java.nio;

public abstract class CharBuffer extends Buffer {

	protected CharBuffer(int capacity) {
		super(capacity);
	}

	public static CharBuffer allocate(int capacity) {
		return new CharBufferArrayBacked(capacity);
	}

	public abstract char get();
	public abstract CharBuffer put(char c);
	public abstract char get(int index);
	public abstract CharBuffer put(int index, char c);

}