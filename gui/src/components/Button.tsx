import React from 'react';
import { cn } from '../lib/utils';

interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'default' | 'destructive' | 'outline' | 'secondary' | 'ghost' | 'accent' | 'info' | 'success' | 'warning';
  size?: 'xs' | 'sm' | 'md' | 'lg' | 'icon';
  children: React.ReactNode;
  loading?: boolean;
}

export function Button({
  variant = 'default',
  size = 'md',
  className,
  children,
  loading = false,
  disabled,
  ...props
}: ButtonProps) {
  return (
    <button
      className={cn(
        'btn border-2',
        {
          'btn-primary border-primary': variant === 'default',
          'btn-error border-error': variant === 'destructive',
          'btn-outline border-base-content/20': variant === 'outline',
          'btn-secondary border-secondary': variant === 'secondary',
          'btn-ghost border-transparent hover:border-base-content/20': variant === 'ghost',
          'btn-accent border-accent': variant === 'accent',
          'btn-info border-info': variant === 'info',
          'btn-success border-success': variant === 'success',
          'btn-warning border-warning': variant === 'warning',
        },
        {
          'btn-xs': size === 'xs',
          'btn-sm': size === 'sm',
          '': size === 'md', // default size, no class needed
          'btn-lg': size === 'lg',
          'btn-square': size === 'icon',
        },
        loading && 'loading',
        className
      )}
      disabled={disabled || loading}
      {...props}
    >
      {loading ? (
        <>
          <span className="loading loading-spinner loading-sm"></span>
          {children}
        </>
      ) : (
        children
      )}
    </button>
  );
}
