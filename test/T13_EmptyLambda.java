public class T13_EmptyLambda {

	public interface I {

		int f(boolean a, float b, int c);

	};

	public static void main(String... args) {
		int someArg0 = 1;
		float someArg1 = 2;
		double someArg2 = 2;

		I r = (a, b, c) -> {
			return (int)((a ? 1 : 0) + b + c + someArg0 + someArg1 + someArg2);
		};
	}

}