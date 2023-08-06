package java.lang;

public class ClassNotFoundException extends ReflectiveOperationException {

	public ClassNotFoundException() {}

	public ClassNotFoundException(String s) {
		super(s);
	}

	public ClassNotFoundException(String s, Throwable ex) {
		super(s, ex);
	}

	public Throwable getException() {
		return super.getCause();
	}

}