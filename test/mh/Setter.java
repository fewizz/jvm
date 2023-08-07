package mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;

public class Setter {

	public static class A {
		int field = 42;
	}

	public static void main(String... args) throws Throwable {
		MethodHandle mh = MethodHandles.publicLookup().findSetter(
			A.class,
			"field",
			int.class
		);
		A a = new A();

		if(a.field != 42) {
			System.exit(1);
		}

		mh.invokeExact(a, 69);

		if(a.field != 69) {
			System.exit(2);
		}
	}

}
