package java.lang;

public final class Class<T> {

	private long ptr_;
	
	public native String getName();

	public native Class<?> getComponentType();

}