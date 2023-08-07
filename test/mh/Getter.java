package mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;

public class Getter {

	public static class A {
		int field = 42;
	}

	public static void main(String... args) throws Throwable {
		MethodHandle mh = MethodHandles.publicLookup().findGetter(
			A.class,
			"field",
			int.class
		);
		A a = new A();
		int v = (int) mh.invokeExact(a);
		if(v != 42) {
			System.exit(1);
		}

		a.field = -1;
		v = (int) mh.invokeExact(a);
		if(v != -1) {
			System.exit(2);
		}
	}

}
