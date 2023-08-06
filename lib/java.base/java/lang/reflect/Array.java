package java.lang.reflect;

public final class Array {
	
	public static native Object newInstance(
		Class<?> componentType, int length
	) throws NegativeArraySizeException;

}