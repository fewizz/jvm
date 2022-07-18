package java.lang.invoke;

public final class StringConcatFactory {

	public static native CallSite makeConcat(
		MethodHandles.Lookup lookup, String name, MethodType concatType
	);

	public static native CallSite makeConcatWithConstants(
		MethodHandles.Lookup lookup, String name, MethodType concatType,
		String recipe, Object... constants
	);

}
