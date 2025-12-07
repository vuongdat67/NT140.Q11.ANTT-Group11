import React from 'react';
import { cn } from '../lib/utils';

interface SelectProps extends Omit<React.SelectHTMLAttributes<HTMLSelectElement>, 'size'> {
  options: Array<{ value: string; label: string }>;
  variant?: 'default' | 'primary' | 'secondary' | 'accent' | 'ghost';
  size?: 'xs' | 'sm' | 'md' | 'lg';
  error?: boolean;
  success?: boolean;
}

export const Select = React.forwardRef<HTMLSelectElement, SelectProps>(
  ({ className, options, variant = 'default', size = 'md', error, success, ...props }, ref) => {
    return (
      <select
        className={cn(
          'select w-full pl-3',
          {
            'select-bordered': variant === 'default',
            'select-primary': variant === 'primary',
            'select-secondary': variant === 'secondary',
            'select-accent': variant === 'accent',
            'select-ghost': variant === 'ghost',
          },
          {
            'select-xs': size === 'xs',
            'select-sm': size === 'sm',
            '': size === 'md', // default size
            'select-lg': size === 'lg',
          },
          error && 'select-error',
          success && 'select-success',
          className
        )}
        ref={ref}
        {...props}
      >
        {options.map((option) => (
          <option key={option.value} value={option.value}>
            {option.label}
          </option>
        ))}
      </select>
    );
  }
);

Select.displayName = 'Select';
