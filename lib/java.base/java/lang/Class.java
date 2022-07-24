package java.lang;

public final class Class<T> {

	private long ptr_;
	
	public String getName() {
		return __getName();
	}

	private native String __getName();

	public native Class<?> getComponentType();

}