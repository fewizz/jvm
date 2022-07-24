package java.util.function;

@FunctionalInterface
public interface Function<T, R> {

	R apply(T t);

	default <V> Function<V,R> compose(Function<? super V,? extends T> before) {
		if(before == null) {
			throw new NullPointerException();
		}
		return (V i) -> apply(before.apply(i));
	}

	default <V> Function<T,V> andThen(Function<? super R,? extends V> after) {
		if(after == null) {
			throw new NullPointerException();
		}
		return (T i) -> after.apply(apply(i));
	}

	static <T> Function<T,T> identity() {
		return (T i) -> i;
	}

}