
public class T10_Interface {

	interface I {
		int meow();
	}

	public static class Impl0 implements I {
		public int meow() {
			return 0;
		}
	}

	public static class Impl1 implements I {
		public int meow() {
			return 1;
		}
	}

	public static void main(String... args) {
		Impl0 i0 = new Impl0();
		Impl1 i1 = new Impl1();

		I i = i0;
		if(i.meow() != 0) {
			System.exit(1);
		}

		i = i1;
		if(i.meow() != 1) {
			System.exit(2);
		}
	}

}