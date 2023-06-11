import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.invoke.MethodHandle;

class T08_MethodHandle {

	static class A {
		public int a() { return 0; }
	}

	static class B extends A {
		@Override
		public int a() { return 1; }
	}

	public static int invokeStatic(int v) {
		return v + 21;
	}

	public static void main(String... args) throws Throwable {
		// static
		MethodHandles.Lookup l = MethodHandles.lookup();
		MethodHandle mh_static = l.findStatic(
			T08_MethodHandle.class,
			"invokeStatic",
			MethodType.methodType(int.class, int.class)
		);
		int result = (int) mh_static.invokeExact(21);

		if(result != 42) {
			System.exit(1);
		}

		// static invoke
		long result_l = (long) mh_static.invoke(Integer.valueOf(42));
		if(result_l != 42 + 21) {
			System.exit(2);
		}

		// virtual
		MethodHandle mh_virtual = l.findVirtual(
			A.class,
			"a",
			MethodType.methodType(int.class)
		);
		result = (int) mh_virtual.invokeExact(new B());
		if(result != 1) {
			System.exit(3);
		}

		// constructor
		MethodHandle mh_constructor = l.findConstructor(
			B.class,
			MethodType.methodType(void.class)
		);
		result = ((B) mh_constructor.invokeExact()).a();
		if(result != 1) {
			System.exit(4);
		}

		// special
		MethodHandle mh_special = l.findSpecial(
			A.class,
			"a",
			MethodType.methodType(int.class),
			B.class
		);
		result = (int) mh_special.invokeExact(new B());
		if(result != 0) {
			System.exit(5);
		}

	}

}