package java.lang;

public class Object {
	
	public Object() {}

	protected Object clone() {
		return __clone();
	}

	private native Object __clone();

	public boolean equals(Object obj) {
		return obj != null && obj == this;
	}

	protected void finalize() {}

	public final Class<?> getClass() {
		return __getClass();
	}

	private native Class<?> __getClass();

	public int hashCode() {
		return __hashCode();
	}

	private native int __hashCode();

	public final void notify() {
		__notify();
	}

	private native void __notify();

	public final void notifyAll() {
		__notifyAll();
	}

	private native void __notifyAll();

	//public String toString() {
	//	return getClass().getName() + '@' + Integer.toHexString(hashCode());
	//}

	public final void wait() {
		__wait();
	}

	private native void __wait();

	public final void wait(long timeoutMillis) {
		__wait(timeoutMillis);
	}

	private native void __wait(long timeoutMillis);

}
