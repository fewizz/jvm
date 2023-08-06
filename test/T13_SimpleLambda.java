public class T13_SimpleLambda {

	public interface I {

		int f(boolean a, float b, int c);

	};

	public static void main(String... args) {
		int someArg0 = 4;
		float someArg1 = 5.0F;
		double someArg2 = 6.0D;

		I r = (a, b, c) -> {
			return (int)((a ? 1 : 0) + b + c + someArg0 + someArg1 + someArg2);
		};

		int result = r.f(true, 2.0F, 3);
		if(result != (int)(1+2.0F+3+4+5.0F+6.0D)) {
			System.exit(1);
		}
	}

}