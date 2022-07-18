package java.lang;

import java.io.Serializable;

public class Throwable implements Serializable {

	private final String message_;
	private Throwable cause_;

	public Throwable() {
		this.message_ = null;
		this.cause_ = null;
	}

	public Throwable(String message) {
		this.message_ = message;
		this.cause_ = null;
	}

	public Throwable(String message, Throwable cause) {
		this.message_ = message;
		this.cause_ = cause;
	}

	public Throwable(Throwable cause) {
		this.message_ = null;
		this.cause_ = cause;
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
		String mess = this.getLocalizedMessage();
		if(mess == null) {
			return mess;
		}
		String className = this.getClass().getName();
		return className + ": " + mess;
	}

}
