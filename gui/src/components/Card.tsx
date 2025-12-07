import React from 'react';
import { cn } from '../lib/utils';

interface CardProps {
  children: React.ReactNode;
  className?: string;
  title?: string;
  description?: string;
  variant?: 'default' | 'bordered' | 'compact';
}

export function Card({ children, className, title, description, variant = 'bordered' }: CardProps) {
  return (
    <div className={cn(
      'card bg-base-200 shadow-lg',
      variant === 'bordered' && 'border border-base-300',
      className
    )}>
      <div className="card-body">
        {(title || description) && (
          <div className="mb-4">
            {title && <h2 className="card-title text-2xl">{title}</h2>}
            {description && <p className="text-sm text-base-content/70 mt-1">{description}</p>}
          </div>
        )}
        <div>{children}</div>
      </div>
    </div>
  );
}
