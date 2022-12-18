public class ThrowRuntimeException {

	static void f() {
		throw new RuntimeException();
	}

	public static void main(String... args) {
		f();
	}

}