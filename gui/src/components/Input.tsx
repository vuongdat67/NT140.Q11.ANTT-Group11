import React from 'react';
import { cn } from '../lib/utils';

interface InputProps extends Omit<React.InputHTMLAttributes<HTMLInputElement>, 'size'> {
  variant?: 'default' | 'primary' | 'secondary' | 'accent' | 'ghost';
  size?: 'xs' | 'sm' | 'md' | 'lg';
  error?: boolean;
  success?: boolean;
}

export const Input = React.forwardRef<HTMLInputElement, InputProps>(
  ({ className, type, variant = 'default', size = 'md', error, success, ...props }, ref) => {
    return (
      <input
        type={type}
        className={cn(
          'input w-full pl-3',
          {
            'input-bordered': variant === 'default',
            'input-primary': variant === 'primary',
            'input-secondary': variant === 'secondary',
            'input-accent': variant === 'accent',
            'input-ghost': variant === 'ghost',
          },
          {
            'input-xs': size === 'xs',
            'input-sm': size === 'sm',
            '': size === 'md', // default size
            'input-lg': size === 'lg',
          },
          error && 'input-error',
          success && 'input-success',
          className
        )}
        ref={ref}
        {...props}
      />
    );
  }
);

Input.displayName = 'Input';
