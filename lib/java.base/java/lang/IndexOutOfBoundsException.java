package java.lang;

public class IndexOutOfBoundsException extends RuntimeException  {

	public IndexOutOfBoundsException() {
		super();
	}

	public IndexOutOfBoundsException(String s) {
		super(s);
	}

	public IndexOutOfBoundsException(int index) {
		super("Illegal index: " + index);
	}

	public IndexOutOfBoundsException(long index) {
		super("Illegal index: " + index);
	}

}