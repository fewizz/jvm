import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.invoke.MethodHandle;

public class AsType {

	static int f(int a, int b, float c) {
		return a + b + (int)c;
	}

	public static void main(String... args) throws Throwable {

		MethodHandle mh = MethodHandles.publicLookup().findStatic(
			AsType.class,
			"f",
			MethodType.methodType(
				int.class,
				int.class, int.class, float.class
			)
		);

		MethodHandle newMh = mh.asType(
			MethodType.methodType(
				float.class,
				Integer.class, short.class, int.class
			)
		);

		float result = (float) newMh.invokeExact(
			Integer.valueOf(1), (short) 2, 3
		);
		if(result != (float)(1 + 2 + (int)(float)3)) {
			System.exit(1);
		}

	}

}