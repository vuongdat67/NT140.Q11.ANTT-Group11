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
        'btn gap-2',
        {
          'btn-primary': variant === 'default',
          'btn-error': variant === 'destructive',
          'btn-outline': variant === 'outline',
          'btn-secondary': variant === 'secondary',
          'btn-ghost': variant === 'ghost',
          'btn-accent': variant === 'accent',
          'btn-info': variant === 'info',
          'btn-success': variant === 'success',
          'btn-warning': variant === 'warning',
        },
        {
          'btn-xs': size === 'xs',
          'btn-sm': size === 'sm',
          '': size === 'md',
          'btn-lg': size === 'lg',
          'btn-square': size === 'icon',
        },
        className
      )}
      disabled={disabled || loading}
      {...props}
    >
      {loading && <span className="loading loading-spinner loading-sm"></span>}
      {children}
    </button>
  );
}
