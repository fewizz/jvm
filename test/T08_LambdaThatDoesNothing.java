import java.util.function.Consumer;

class T08_LambdaThatDoesNothing {

	public static void main(String... args) {
		Consumer<Integer> l = i -> { return; };
		l.accept(0);
	}

}