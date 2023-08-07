package mh;

import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public class Virtual {

	public static class A {
		public int someMethod(int a) {
			return a + 12;
		}
	}

	public static void main(String... args)
		throws Throwable
	{
		MethodHandle mh = MethodHandles.publicLookup().findVirtual(
			A.class,
			"someMethod",
			MethodType.methodType(int.class, int.class)
		);

		A a = new A();

		int result = (int) mh.invokeExact(a, 42 - 12);
		if(result != 42) {
			System.exit(-1);
		}
	}

}
