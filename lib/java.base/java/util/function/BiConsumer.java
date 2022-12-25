package java.util.function;

@FunctionalInterface
public interface BiConsumer<T, U> {

	void accept(T t, U u);

	default BiConsumer<T, U> andThen(BiConsumer<? super T, ? super U> after) {
		if(after == null) {
			throw new NullPointerException();
		}

		BiConsumer<T, U> before = this;

		return new BiConsumer<T, U>() {
			@Override
			public void accept(T t, U u) {
				before.accept(t, u);
				after.accept(t, u);
			}
		};
	}

}