package java.lang;

public class Object {
	
	public Object() {}

	public final native Class<?> getClass();

	public native int hashCode();

	public boolean equals(Object obj) {
		return obj != null && obj == this;
	}

	protected native Object clone();

	public String toString() {
		return getClass().getName() + '@' + Integer.toHexString(hashCode());
	}

	public final native void notify();
	public final native void notifyAll();

	public final native void wait(); // TODO throws
	public final native void wait(long timeoutMillis);
	public final native void wait(long timeoutMillis, int nanos);

	protected void finalize() {}

	//public String toString() {
	//	return getClass().getName() + '@' + Integer.toHexString(hashCode());
	//}

}
