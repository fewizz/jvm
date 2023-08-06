package java.util.function;

@FunctionalInterface
public interface Function<T, R> {

	R apply(T t);

	default <V> Function<V, R> compose(
		Function<? super V, ? extends T> before
	) {
		if(before == null) {
			throw new NullPointerException();
		}
		Function<T, R> after = this;
		return new Function<V, R>() {
			@Override
			public R apply(V v) {
				return after.apply(before.apply(v));
			}
		};
	}

	default <V> Function<T, V> andThen(Function<? super R, ? extends V> after) {
		if(after == null) {
			throw new NullPointerException();
		}
		Function<T, R> before = this;
		return new Function<T, V>() {
			@Override
			public V apply(T t) {
				return after.apply(before.apply(t));
			}
		};
	}

	static <T> Function<T, T> identity() {
		return new Function<T, T>() {
			@Override
			public T apply(T t) {
				return t;
			}
		};
	}

}