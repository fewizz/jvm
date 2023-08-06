package java.util.function;

@FunctionalInterface
public interface Consumer<T> {

	void accept(T t);

	default Consumer<T> andThen(Consumer<? super T> after) {
		if(after == null) {
			throw new NullPointerException();
		}

		Consumer<T> before = this;

		return new Consumer<T>() {
			@Override
			public void accept(T t) {
				before.accept(t);
				after.accept(t);
			}
		};
	}

}