package java.util;

import java.util.function.UnaryOperator;

public interface List<E> extends Collection<E> {

	@Override
	int size();

	@Override
	boolean isEmpty();

	@Override
	boolean contains(Object o);

	@Override
	Iterator<E> iterator();

	@Override
	Object[] toArray();

	<T> T[] toArray(T[] a);

	@Override
	boolean add(E e);

	@Override
	boolean remove(Object o);

	@Override
	boolean containsAll(Collection<?> c);

	@Override
	boolean addAll(Collection<? extends E> c);

	boolean addAll(int index, Collection<? extends E> c);

	@Override
	boolean removeAll(Collection<?> c);

	boolean retainAll(Collection<?> c);

	default void replaceAll(UnaryOperator<E> operator) {
		if(operator == null) {
			throw new NullPointerException();
		}
		ListIterator<E> it = listIterator();
		while(it.hasNext()) {
			it.set(operator.apply(it.next()));
		}
	}

	@SuppressWarnings({"unchecked"})
	default void sort(Comparator<? super E> c) {
		Object[] a = toArray();
		Arrays.sort(a, (Comparator<Object>) c);
		ListIterator<E> it = listIterator();
		for(Object o : a) {
			it.next();
			it.set((E) o);
		}
	}

	@Override
	void clear();

	@Override
	boolean equals(Object o);

	@Override
	int hashCode();

	E get(int index);

	E set(int index, E element);

	void add(int index, E element);

	E remove(int index);

	int indexOf(Object o);

	int lastIndexOf(Object o);

	ListIterator<E> listIterator();

	ListIterator<E> listIterator(int index);

	List<E> subList(int fromIndex, int toIndex);

	default Spliterator<E> spliterator() {
		throw new UnsupportedOperationException(); // TODO
	}



}