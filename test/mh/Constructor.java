package mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class Constructor {

	public static class A {
		final int v;
		final float f;

		public A(int v, float f) {
			this.v = v;
			this.f = f;
		}
	}

	public static void main(String... args) throws Throwable {
		MethodHandle mh = MethodHandles.publicLookup().findConstructor(
			A.class, MethodType.methodType(void.class, int.class, float.class)
		);
		A result = (A)mh.invokeExact(42, -1.0F);
		if(result.v != 42) {
			System.exit(1);
		}
		if(result.f != -1.0F) {
			System.exit(2);
		}
	}

}
