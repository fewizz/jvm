import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class Static {

	public static void main(String... args)
		throws Throwable
	{
		MethodHandle mh = MethodHandles.publicLookup().findStatic(
			SomeClass.class,
			"someStaticMethod",
			MethodType.methodType(long.class, int.class, int.class, int.class)
		);
		long result = (long) mh.invokeExact(10, -2, -8);
		if(result != 0) System.exit(-1);
	}

	public static class SomeClass {

		public static long someStaticMethod(int a, int b, int c) {
			return a + b + c;
		}

	}

}
