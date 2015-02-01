#! /usr/bin/env runhaskell

{-# LANGUAGE LambdaCase #-}

import System.Directory (doesFileExist, getDirectoryContents)
import System.FilePath ((</>))

import Data.String.Utils
import Data.List


-- Util methods

mapSnd :: (a -> b) -> [(c, a)] -> [(c, b)]
mapSnd f xys = map ( \case (x, y) -> (x, f y) ) xys

mapDir :: (FilePath -> IO ()) -> FilePath -> IO ()
mapDir proc fp = do
        isFile <- doesFileExist fp -- is a file of fp
        if isFile then proc fp -- process the file
                  else getDirectoryContents fp >>=
                  mapM_ (mapDir proc . (fp </>)) . filter (`notElem` [".", ".."])

main :: IO ()
main = mapDir process "src"


-- Parsing methods

extractBlock :: [String] -> [String]
extractBlock =
        takeWhile (startswith "//")

isTodoBlock :: (Integer, [String]) -> Bool
isTodoBlock (_, block) =
        (not $ null block) && (
            (startswith "// TODO: " $ head block) ||
            (startswith "// NOTE: " $ head block)
        )

joinBlock :: [String] -> String
joinBlock block =
        unlines $
        map ( \line ->
            ( dropWhile (== '/') line )
        ) $
        block


-- File processing

process :: FilePath -> IO ()
process filename = do
        -- Getting the file contents
        content <- readFile filename

        -- Items with line numbers
        let items :: [(Integer, [String])]
            items =
                zip [1..] $
                tails $
                map strip $
                lines content

        -- Only those starting with TODO
        let todoBlocks :: [(Integer, [String])]
            todoBlocks =
                -- Getting only the comment block
                mapSnd extractBlock $
                -- Getting comment blocks that define a todo item
                filter isTodoBlock $
                items

        -- Todo items
        let todoItems :: [(Integer, String)]
            todoItems =
                -- Getting the item blocks into actual items
                mapSnd joinBlock todoBlocks


        if (not $ null todoItems)
            then
                putStrLn $
                    concat $
                    map (\case (lineNo, todoItem) ->
                                filename ++ ":" ++ (show lineNo) ++ ": " ++ todoItem
                        ) todoItems
            else
                return ()


