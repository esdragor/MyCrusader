// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of the simple formation (J7a, J7-Q3), on the ground plane (cm). A group's move order makes it arrive in
 * rows across the march: how wide the rows are (one shape for Peasants, a square for one kind of soldier, odd rows for
 * several kinds, or the width the right-button drag gives), which kind of soldier stands where (the kind in front
 * first, a shared row with the new kind in its centre and the kind that ends there on its wings), where the places
 * are in the world, and which unit goes to which place without two paths crossing. No engine state: tested on their
 * own.
 */
namespace HOLFormation
{
	/**
	 * Rows of Peasants (the user's rule, an exception: soldiers never use it): up to 5, one row; beyond, rows of
	 * ceil(sqrt(2 N)), full from the front, the last one shorter (10 -> 5, 5; 15 -> 6, 6, 3; 20 -> 7, 7, 6).
	 * OutWidths holds the number of places of each row, front first.
	 */
	HEROESOFLIBERTY_API void MakePeasantRows(int32 Count, TArray<int32>& OutWidths);

	/** Rows of a single kind of soldier: a square of ceil(sqrt(N)), the last row shorter (12 -> 4, 4, 4; 20 -> 5 x 4). */
	HEROESOFLIBERTY_API void MakeSquareRows(int32 Count, TArray<int32>& OutWidths);

	/**
	 * Rows of several kinds of soldiers: W the smallest odd number at least sqrt(N), R = N / W rows of W, a remainder
	 * e = N - R W. When R >= 2 and e <= R, the e / 2 first rows take W + 2 and, e odd, the next one W + 1; otherwise one
	 * more row of e (24 -> 7, 7, 5, 5; 40 -> 9, 9, 8, 7, 7; 60 -> 11, 11, 11, 9, 9, 9; 4 -> 3, 1).
	 */
	HEROESOFLIBERTY_API void MakeMixedRows(int32 Count, TArray<int32>& OutWidths);

	/** Rows of a width the player gave (the right-button drag, in columns): full rows, the last one shorter. */
	HEROESOFLIBERTY_API void MakeRowsOfWidth(int32 Count, int32 Width, TArray<int32>& OutWidths);

	/**
	 * Which kind stands on each place. CountOfKind holds the number of units of each kind, sorted by row priority (the
	 * kind in front first: Spearman, then Man-at-arms, then Archer). The kinds fill the rows in that order, front to
	 * back; in a row, the places are taken from the centre outwards by the kinds that start in it (by priority), then
	 * by the kind that ends in it (started in a row in front), on the wings; an odd wing has its extra unit on the left.
	 * OutKindOfPlace is row by row, front first, each row from left to right; it holds an index into CountOfKind
	 * (INDEX_NONE on a place left over when the rows hold more places than units).
	 */
	HEROESOFLIBERTY_API void PlaceKinds(TConstArrayView<int32> RowWidths, TConstArrayView<int32> CountOfKind, TArray<int32>& OutKindOfPlace);

	/**
	 * The places in the world, in the order of PlaceKinds (row by row, front first, each from left to right). Target is
	 * the middle of the front row (the point clicked, or the middle of the drag: the front stands there); Forward is
	 * the march's direction (normalised here). Each row is centred on the march's axis, ColumnSpacing between two
	 * places of a row (an even row has its places at +-0.5 spacing), RowSpacing between two rows.
	 */
	HEROESOFLIBERTY_API void MakePlaces(TConstArrayView<int32> RowWidths, const FVector2D& Target, const FVector2D& Forward,
		double ColumnSpacing, double RowSpacing, TArray<FVector2D>& OutPlaces);

	/**
	 * Gives the units of one kind the places of that kind so that no two straight paths cross (D230: two units whose
	 * paths cross go through each other): units and places paired by their order across the march (and along it on a
	 * tie), then any two units whose swap makes the paths shorter in total swap, until none does; two crossing paths
	 * always get shorter once swapped, so none is left. Positions and Places are world points (as many places as units,
	 * or more: the extra ones stay free). OutPlaceOfUnit[i] is the index in Places given to unit i.
	 */
	HEROESOFLIBERTY_API void AssignPlaces(TConstArrayView<FVector2D> Positions, TConstArrayView<FVector2D> Places, const FVector2D& Forward,
		TArray<int32>& OutPlaceOfUnit);
}
