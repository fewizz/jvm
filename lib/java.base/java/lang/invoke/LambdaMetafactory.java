package java.lang.invoke;

public final class LambdaMetafactory {
	
	public static CallSite metafactory(
		MethodHandles.Lookup caller, String interfaceMethodName,
		MethodType factoryType, MethodType interfaceMethodType,
		MethodHandle implementation, MethodType dynamicMethodType
	) {
		return null;
	}

}