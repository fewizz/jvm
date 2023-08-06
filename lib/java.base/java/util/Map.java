package java.util;

import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.Function;

public interface Map<K,V> {

	public static interface Entry<K,V> {

		K getKey();

		V getValue();

		V setValue(V value);

		boolean equals(Object o);

		int hashCode();

	}

	int size();

	boolean isEmpty();

	boolean containsKey(Object key);

	boolean containsValue(Object value);

	V get(Object key);

	V put(K key, V value);

	V remove(Object key);

	//void putAll(Map<? extends K,? extends V> m);

	void clear();

	//Set<K> keySet();

	//Collection<V> values();

	//Set<Map.Entry<K,V>> entrySet();

	boolean equals(Object o);

	int hashCode();

	default V getOrDefault(Object key, V defaultValue) {
		V v = this.get(key);
		return v != null || containsKey(key)? v : defaultValue;
	}

	/*default void forEach(BiConsumer<? super K,? super V> action) {
		if(action == null) throw new NullPointerException();

		for (Map.Entry<K, V> entry : this.entrySet()) {
			action.accept(entry.getKey(), entry.getValue());
		}
	}*/

	//default void replaceAll(
	//	BiFunction<? super K,? super V,? extends V> function
	//); // TODO

	default V putIfAbsent(K key, V value) {
		V v = this.get(key);
		if (v == null) {
			v = this.put(key, value);
		}
		return v;
	}

	default boolean remove(Object key, Object value) {
		if (this.containsKey(key) && Objects.equals(this.get(key), value)) {
			this.remove(key);
			return true;
		} else {
			return false;
		}
	}

	default boolean replace(K key, V oldValue, V newValue) {
		if (this.containsKey(key) && Objects.equals(this.get(key), oldValue)) {
			this.put(key, newValue);
			return true;
		} else {
			return false;
		}
	}

	default V replace(K key, V value) {
		if (this.containsKey(key)) {
			return this.put(key, value);
		} else {
			return null;
		}
	}

	default V computeIfAbsent(
		K key, Function<? super K,? extends V> mappingFunction
	) {
		V result = this.get(key);
		if (result == null) {
			result = mappingFunction.apply(key);
			if (result != null) {
				this.put(key, result);
			}
		}
		return result;
	}

	default V computeIfPresent(
		K key, BiFunction<? super K,? super V,? extends V> remappingFunction
	) {
		V result = this.get(key); 
		if (result != null) {
			V oldValue = result;
			result = remappingFunction.apply(key, oldValue);
			if (result != null) {
				this.put(key, result);
			}
			else {
				this.remove(key);
			}
		}
		return result;
	}
}