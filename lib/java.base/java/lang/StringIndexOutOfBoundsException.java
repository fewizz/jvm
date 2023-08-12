package java.lang;

public class StringIndexOutOfBoundsException extends IndexOutOfBoundsException {

	public StringIndexOutOfBoundsException() {}

	public StringIndexOutOfBoundsException(String s) { super(s); }

	public StringIndexOutOfBoundsException(int index) {
		this("illegal index: " + index);
	}

}
