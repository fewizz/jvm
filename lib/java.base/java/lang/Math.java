package java.lang;

public final class Math {

	public static int max(int a, int b) {
		return a > b ? a : b;
	}

	public static double min(double a, double b) {
		return Double.min(a, b);
	}

	public static int min(int a, int b) {
		return a < b ? a : b;
	}

}