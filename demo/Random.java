import java.util.Scanner;

public class Random {

	public static void main(String... args) {
		java.util.Random r = new java.util.Random();
		Scanner s = new Scanner(System.in);

		while(true) {
			s.next();
			System.out.println("" + r.nextInt(4));
		}
	}

}
