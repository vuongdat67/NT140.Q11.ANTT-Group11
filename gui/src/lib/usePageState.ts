import { useState, useEffect } from 'react';

/**
 * Hook to persist page state in localStorage
 * Prevents state from being cleared when navigating away
 */
export function usePageState<T>(
  key: string,
  initialState: T
): [T, (value: T | ((prev: T) => T)) => void] {
  // Get initial state from localStorage or use provided initial state
  const getInitialState = (): T => {
    try {
      const stored = localStorage.getItem(`pageState_${key}`);
      if (stored) {
        return JSON.parse(stored);
      }
    } catch (e) {
      console.error(`Error loading page state for ${key}:`, e);
    }
    return initialState;
  };

  const [state, setState] = useState<T>(getInitialState);

  // Save to localStorage whenever state changes
  useEffect(() => {
    try {
      localStorage.setItem(`pageState_${key}`, JSON.stringify(state));
    } catch (e) {
      console.error(`Error saving page state for ${key}:`, e);
    }
  }, [key, state]);

  // Load from localStorage when component mounts
  useEffect(() => {
    const stored = localStorage.getItem(`pageState_${key}`);
    if (stored) {
      try {
        const parsed = JSON.parse(stored);
        setState(parsed);
      } catch (e) {
        console.error(`Error parsing page state for ${key}:`, e);
      }
    }
  }, [key]);

  return [state, setState];
}

/**
 * Clear page state from localStorage
 */
export function clearPageState(key: string) {
  localStorage.removeItem(`pageState_${key}`);
}

