package java.lang;

import java.io.Serializable;

public class Throwable implements Serializable {

	private final String message_;
	private Throwable cause_;
	private StackTraceElement[] stackTrace_;

	public Throwable() {
		this.message_ = null;
		this.cause_ = null;
		fillInStackTrace();
	}

	public Throwable(String message) {
		this.message_ = message;
		this.cause_ = null;
		fillInStackTrace();
	}

	public Throwable(String message, Throwable cause) {
		this.message_ = message;
		this.cause_ = cause;
		fillInStackTrace();
	}

	public Throwable(Throwable cause) {
		this.message_ = cause == null ? null : cause.toString();
		this.cause_ = cause;
		fillInStackTrace();
	}

	public String getMessage() {
		return this.message_;
	}

	public String getLocalizedMessage() {
		return this.message_;
	}

	public Throwable getCause() {
		return this.cause_;
	}

	public Throwable initCause(Throwable cause) {
		this.cause_ = cause;
		return this;
	}

	@Override
	public java.lang.String toString() {
		StringBuilder sb = new StringBuilder();

		String className = this.getClass().getName();
		sb.append(className);

		String mess = this.getLocalizedMessage();

		if(mess != null) {
			sb.append(": ");
			sb.append(mess);
		}

		return sb.toString();
	}

	public void printStackTrace() {
		System.err.println(toString());
		for(StackTraceElement e : stackTrace_) {
			System.err.print('\t');
			System.err.println(e.toString());
		}
	}

	public native Throwable fillInStackTrace();

}
