import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public class Special {

	public static class A {
		public int f() {
			return 1;
		}
	}

	public static class B extends A {
		@Override
		public int f() {
			return 2;
		}

		public static MethodHandles.Lookup lookup() {
			return MethodHandles.lookup();
		}
	}

	public static class C extends B {
		@Override
		public int f() {
			return 3;
		}
	}

	public static void main(String... args) throws Throwable {
		MethodHandle mh = B.lookup().findSpecial(
			A.class,
			"f",
			MethodType.methodType(int.class),
			B.class
		);

		B b = new B();
		int result = (int) mh.invokeExact(b);

		if(result != 1) {
			System.exit(1);
		}
	}

}
