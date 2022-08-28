import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.invoke.MethodHandle;

class T08_MethodHandle {

	public static int invokeStatic(int v) {
		return v + 21;
	}

	public static void main(String... args) throws Throwable {
		MethodHandles.Lookup l = MethodHandles.lookup();
		MethodHandle mh = l.findStatic(
			T08_MethodHandle.class,
			"invokeStatic",
			MethodType.methodType(int.class, int.class)
		);
		int result = (int) mh.invokeExact(21);

		if(result != 42) {
			System.exit(1);
		}
	}

}