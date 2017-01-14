/**CFile****************************************************************

  FileName    [giaShow.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [And-Inverter Graph package.]

  Synopsis    [AIG visualization.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - May 11, 2006.]

  Revision    [$Id: giaShow.c,v 1.00 2006/05/11 00:00:00 alanmi Exp $]

***********************************************************************/

#include "gia.h"
#include "proof/cec/cec.h"
#include "proof/acec/acec.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Writes the graph structure of AIG for DOT.]

  Description [Useful for graph visualization using tools such as GraphViz: 
  http://www.graphviz.org/]
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_WriteDotAigSimple( Gia_Man_t * p, char * pFileName, Vec_Int_t * vBold )
{
    FILE * pFile;
    Gia_Obj_t * pNode;//, * pTemp, * pPrev;
    int LevelMax, Prev, Level, i;
    int fConstIsUsed = 0;

    if ( Gia_ManAndNum(p) > 500 )
    {
        fprintf( stdout, "Cannot visualize AIG with more than 500 nodes.\n" );
        return;
    }
    if ( (pFile = fopen( pFileName, "w" )) == NULL )
    {
        fprintf( stdout, "Cannot open the intermediate file \"%s\".\n", pFileName );
        return;
    }

    // mark the nodes
    if ( vBold )
        Gia_ManForEachObjVec( vBold, p, pNode, i )
            pNode->fMark0 = 1;
    else if ( p->nXors || p->nMuxes )
        Gia_ManForEachObj( p, pNode, i )
            if ( Gia_ObjIsXor(pNode) || Gia_ObjIsMux(p, pNode) )
                pNode->fMark0 = 1;

    // compute levels
    LevelMax = 1 + Gia_ManLevelNum( p );
    Gia_ManForEachCo( p, pNode, i )
        Vec_IntWriteEntry( p->vLevels, Gia_ObjId(p, pNode), LevelMax );

    // write the DOT header
    fprintf( pFile, "# %s\n",  "AIG structure generated by IVY package" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "digraph AIG {\n" );
    fprintf( pFile, "size = \"7.5,10\";\n" );
//  fprintf( pFile, "ranksep = 0.5;\n" );
//  fprintf( pFile, "nodesep = 0.5;\n" );
    fprintf( pFile, "center = true;\n" );
//  fprintf( pFile, "orientation = landscape;\n" );
//  fprintf( pFile, "edge [fontsize = 10];\n" );
//  fprintf( pFile, "edge [dir = none];\n" );
    fprintf( pFile, "edge [dir = back];\n" );
    fprintf( pFile, "\n" );

    // labels on the left of the picture
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  node [shape = plaintext];\n" );
    fprintf( pFile, "  edge [style = invis];\n" );
    fprintf( pFile, "  LevelTitle1 [label=\"\"];\n" );
    fprintf( pFile, "  LevelTitle2 [label=\"\"];\n" );
    // generate node names with labels
    for ( Level = LevelMax; Level >= 0; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d", Level );
        fprintf( pFile, " [label = " );
        // label name
        fprintf( pFile, "\"" );
        fprintf( pFile, "\"" );
        fprintf( pFile, "];\n" );
    }

    // genetate the sequence of visible/invisible nodes to mark levels
    fprintf( pFile, "  LevelTitle1 ->  LevelTitle2 ->" );
    for ( Level = LevelMax; Level >= 0; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d",  Level );
        // the connector
        if ( Level != 0 )
            fprintf( pFile, " ->" );
        else
            fprintf( pFile, ";" );
    }
    fprintf( pFile, "\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate title box on top
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle1;\n" );
    fprintf( pFile, "  title1 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=20,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    fprintf( pFile, "%s", "AIG structure visualized by ABC" );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "Benchmark \\\"%s\\\". ", "aig" );
//    fprintf( pFile, "Time was %s. ",  Extra_TimeStamp() );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate statistics box
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle2;\n" );
    fprintf( pFile, "  title2 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=18,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    fprintf( pFile, "The set contains %d logic nodes and spans %d levels.", Gia_ManAndNum(p), LevelMax );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate the COs
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    // the labeling node of this level
    fprintf( pFile, "  Level%d;\n",  LevelMax );
    // generate the CO nodes
    Gia_ManForEachCo( p, pNode, i )
    {
        if ( Gia_ObjFaninId0p(p, pNode) == 0 )
            fConstIsUsed = 1;
/*
        if ( fHaig || pNode->pEquiv == NULL )
            fprintf( pFile, "  Node%d%s [label = \"%d%s\"", pNode->Id, 
                (Gia_ObjIsLatch(pNode)? "_in":""), pNode->Id, (Gia_ObjIsLatch(pNode)? "_in":"") );
        else
            fprintf( pFile, "  Node%d%s [label = \"%d%s(%d%s)\"", pNode->Id, 
                (Gia_ObjIsLatch(pNode)? "_in":""), pNode->Id, (Gia_ObjIsLatch(pNode)? "_in":""), 
                    Gia_Regular(pNode->pEquiv)->Id, Gia_IsComplement(pNode->pEquiv)? "\'":"" );
*/
        fprintf( pFile, "  Node%d [label = \"%d\"", Gia_ObjId(p, pNode), Gia_ObjId(p, pNode) ); 

        fprintf( pFile, ", shape = %s", "invtriangle" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate nodes of each rank
    for ( Level = LevelMax - 1; Level > 0; Level-- )
    {
        fprintf( pFile, "{\n" );
        fprintf( pFile, "  rank = same;\n" );
        // the labeling node of this level
        fprintf( pFile, "  Level%d;\n",  Level );
        Gia_ManForEachObj( p, pNode, i )
        {
            if ( (int)Gia_ObjLevel(p, pNode) != Level )
                continue;
/*
            if ( fHaig || pNode->pEquiv == NULL )
                fprintf( pFile, "  Node%d [label = \"%d\"", pNode->Id, pNode->Id );
            else 
                fprintf( pFile, "  Node%d [label = \"%d(%d%s)\"", pNode->Id, pNode->Id, 
                    Gia_Regular(pNode->pEquiv)->Id, Gia_IsComplement(pNode->pEquiv)? "\'":"" );
*/
            fprintf( pFile, "  Node%d [label = \"%d\"", i, i ); 

            if ( Gia_ObjIsXor(pNode) )
                fprintf( pFile, ", shape = doublecircle" );
            else if ( Gia_ObjIsMux(p, pNode) )
                fprintf( pFile, ", shape = trapezium" );
            else
                fprintf( pFile, ", shape = ellipse" );

            if ( pNode->fMark0 )
                fprintf( pFile, ", style = filled" );
            fprintf( pFile, "];\n" );
        }
        fprintf( pFile, "}" );
        fprintf( pFile, "\n" );
        fprintf( pFile, "\n" );
    }

    // generate the CI nodes
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    // the labeling node of this level
    fprintf( pFile, "  Level%d;\n",  0 );
    // generate constant node
    if ( fConstIsUsed )
    {
        // check if the costant node is present
        fprintf( pFile, "  Node%d [label = \"Const0\"", 0 );
        fprintf( pFile, ", shape = ellipse" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    // generate the CI nodes
    Gia_ManForEachCi( p, pNode, i )
    {
/*
        if ( fHaig || pNode->pEquiv == NULL )
            fprintf( pFile, "  Node%d%s [label = \"%d%s\"", pNode->Id, 
                (Gia_ObjIsLatch(pNode)? "_out":""), pNode->Id, (Gia_ObjIsLatch(pNode)? "_out":"") );
        else
            fprintf( pFile, "  Node%d%s [label = \"%d%s(%d%s)\"", pNode->Id, 
                (Gia_ObjIsLatch(pNode)? "_out":""), pNode->Id, (Gia_ObjIsLatch(pNode)? "_out":""), 
                    Gia_Regular(pNode->pEquiv)->Id, Gia_IsComplement(pNode->pEquiv)? "\'":"" );
*/
        fprintf( pFile, "  Node%d [label = \"%d\"", Gia_ObjId(p, pNode), Gia_ObjId(p, pNode) ); 

        fprintf( pFile, ", shape = %s", "triangle" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate invisible edges from the square down
    fprintf( pFile, "title1 -> title2 [style = invis];\n" );
    Gia_ManForEachCo( p, pNode, i )
        fprintf( pFile, "title2 -> Node%d [style = invis];\n", Gia_ObjId(p, pNode) );
    // generate invisible edges among the COs
    Prev = -1;
    Gia_ManForEachCo( p, pNode, i )
    {
        if ( i > 0 )
            fprintf( pFile, "Node%d -> Node%d [style = invis];\n", Prev, Gia_ObjId(p, pNode) );
        Prev = Gia_ObjId(p, pNode);
    }
    // generate invisible edges among the CIs
    Prev = -1;
    Gia_ManForEachCi( p, pNode, i )
    {
        if ( i > 0 )
            fprintf( pFile, "Node%d -> Node%d [style = invis];\n", Prev, Gia_ObjId(p, pNode) );
        Prev = Gia_ObjId(p, pNode);
    }

    // generate edges
    Gia_ManForEachObj( p, pNode, i )
    {
        if ( !Gia_ObjIsAnd(pNode) && !Gia_ObjIsCo(pNode) && !Gia_ObjIsBuf(pNode) )
            continue;
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  i );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ObjFaninId0(pNode, i) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC0(pNode)? "dotted" : "bold" );
//        if ( Gia_NtkIsSeq(pNode->p) && Seq_ObjFaninL0(pNode) > 0 )
//        fprintf( pFile, ", label = \"%s\"", Seq_ObjFaninGetInitPrintable(pNode,0) );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );
        if ( !Gia_ObjIsAnd(pNode) )
            continue;
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  i );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ObjFaninId1(pNode, i) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC1(pNode)? "dotted" : "bold" );
//        if ( Gia_NtkIsSeq(pNode->p) && Seq_ObjFaninL1(pNode) > 0 )
//        fprintf( pFile, ", label = \"%s\"", Seq_ObjFaninGetInitPrintable(pNode,1) );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );

        if ( !Gia_ObjIsMux(p, pNode) )
            continue;
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  i );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ObjFaninId2(p, i) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC2(p, pNode)? "dotted" : "bold" );
//        if ( Gia_NtkIsSeq(pNode->p) && Seq_ObjFaninL1(pNode) > 0 )
//        fprintf( pFile, ", label = \"%s\"", Seq_ObjFaninGetInitPrintable(pNode,1) );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );

/*
        // generate the edges between the equivalent nodes
        if ( fHaig && pNode->pEquiv && Gia_ObjRefs(pNode) > 0 )
        {
            pPrev = pNode;
            for ( pTemp = pNode->pEquiv; pTemp != pNode; pTemp = Gia_Regular(pTemp->pEquiv) )
            {
                fprintf( pFile, "Node%d",  pPrev->Id );
                fprintf( pFile, " -> " );
                fprintf( pFile, "Node%d",  pTemp->Id );
                fprintf( pFile, " [style = %s]", Gia_IsComplement(pTemp->pEquiv)? "dotted" : "bold" );
                fprintf( pFile, ";\n" );
                pPrev = pTemp;
            }
            // connect the last node with the first
            fprintf( pFile, "Node%d",  pPrev->Id );
            fprintf( pFile, " -> " );
            fprintf( pFile, "Node%d",  pNode->Id );
            fprintf( pFile, " [style = %s]", Gia_IsComplement(pPrev->pEquiv)? "dotted" : "bold" );
            fprintf( pFile, ";\n" );
        }
*/
    }

    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );
    fclose( pFile );

    // unmark nodes
    if ( vBold )
        Gia_ManForEachObjVec( vBold, p, pNode, i )
            pNode->fMark0 = 0;
    else if ( p->nXors || p->nMuxes )
        Gia_ManCleanMark0( p );

    Vec_IntFreeP( &p->vLevels );
}

/**Function*************************************************************

  Synopsis    [Writes the graph structure of AIG for DOT.]

  Description [Useful for graph visualization using tools such as GraphViz: 
  http://www.graphviz.org/]
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ShowAddOut( Vec_Int_t * vAdds, Vec_Int_t * vMapAdds, int Node )
{
    int iBox = Vec_IntEntry( vMapAdds, Node );
    if ( iBox >= 0 )
        return Vec_IntEntry( vAdds, 6*iBox+4 );
    return Node;
}
void Gia_WriteDotAig( Gia_Man_t * p, char * pFileName, Vec_Int_t * vAdds, Vec_Int_t * vXors, Vec_Int_t * vMapAdds, Vec_Int_t * vMapXors, Vec_Int_t * vOrder )
{
    FILE * pFile;
    Gia_Obj_t * pNode;//, * pTemp, * pPrev;
    int LevelMax, Prev, Level, i;
    int fConstIsUsed = 0;

    if ( Gia_ManAndNum(p) > 1000 )
    {
        fprintf( stdout, "Cannot visualize AIG with more than 1000 nodes.\n" );
        return;
    }
    if ( (pFile = fopen( pFileName, "w" )) == NULL )
    {
        fprintf( stdout, "Cannot open the intermediate file \"%s\".\n", pFileName );
        return;
    }

    // compute levels
    LevelMax = 1 + p->nLevels;
    Gia_ManForEachCo( p, pNode, i )
        Vec_IntWriteEntry( p->vLevels, Gia_ObjId(p, pNode), LevelMax );

    // write the DOT header
    fprintf( pFile, "# %s\n",  "AIG structure generated by IVY package" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "digraph AIG {\n" );
    fprintf( pFile, "size = \"7.5,10\";\n" );
//  fprintf( pFile, "ranksep = 0.5;\n" );
//  fprintf( pFile, "nodesep = 0.5;\n" );
    fprintf( pFile, "center = true;\n" );
//  fprintf( pFile, "orientation = landscape;\n" );
//  fprintf( pFile, "edge [fontsize = 10];\n" );
//  fprintf( pFile, "edge [dir = none];\n" );
    fprintf( pFile, "edge [dir = back];\n" );
    fprintf( pFile, "\n" );

    // labels on the left of the picture
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  node [shape = plaintext];\n" );
    fprintf( pFile, "  edge [style = invis];\n" );
    fprintf( pFile, "  LevelTitle1 [label=\"\"];\n" );
    fprintf( pFile, "  LevelTitle2 [label=\"\"];\n" );
    // generate node names with labels
    for ( Level = LevelMax; Level >= 0; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d", Level );
        fprintf( pFile, " [label = " );
        // label name
        fprintf( pFile, "\"" );
        fprintf( pFile, "\"" );
        fprintf( pFile, "];\n" );
    }

    // genetate the sequence of visible/invisible nodes to mark levels
    fprintf( pFile, "  LevelTitle1 ->  LevelTitle2 ->" );
    for ( Level = LevelMax; Level >= 0; Level-- )
    {
        // the visible node name
        fprintf( pFile, "  Level%d",  Level );
        // the connector
        if ( Level != 0 )
            fprintf( pFile, " ->" );
        else
            fprintf( pFile, ";" );
    }
    fprintf( pFile, "\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate title box on top
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle1;\n" );
    fprintf( pFile, "  title1 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=20,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    fprintf( pFile, "%s", "AIG structure visualized by ABC" );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "Benchmark \\\"%s\\\". ", "aig" );
//    fprintf( pFile, "Time was %s. ",  Extra_TimeStamp() );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate statistics box
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    fprintf( pFile, "  LevelTitle2;\n" );
    fprintf( pFile, "  title2 [shape=plaintext,\n" );
    fprintf( pFile, "          fontsize=18,\n" );
    fprintf( pFile, "          fontname = \"Times-Roman\",\n" );
    fprintf( pFile, "          label=\"" );
    fprintf( pFile, "The set contains %d logic nodes and spans %d levels.", Gia_ManAndNum(p), LevelMax );
    fprintf( pFile, "\\n" );
    fprintf( pFile, "\"\n" );
    fprintf( pFile, "         ];\n" );
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate the COs
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    // the labeling node of this level
    fprintf( pFile, "  Level%d;\n",  LevelMax );
    // generate the CO nodes
    Gia_ManForEachCo( p, pNode, i )
    {
        if ( Gia_ObjFaninId0p(p, pNode) == 0 )
            fConstIsUsed = 1;
        fprintf( pFile, "  Node%d [label = \"%d\"", Gia_ObjId(p, pNode), Gia_ObjId(p, pNode) ); 

        fprintf( pFile, ", shape = %s", "invtriangle" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate nodes of each rank
    for ( Level = LevelMax - 1; Level > 0; Level-- )
    {
        fprintf( pFile, "{\n" );
        fprintf( pFile, "  rank = same;\n" );
        // the labeling node of this level
        fprintf( pFile, "  Level%d;\n",  Level );
        Gia_ManForEachObjVec( vOrder, p, pNode, i )
        {
            int iNode = Gia_ObjId( p, pNode );
            if ( (int)Gia_ObjLevel(p, pNode) != Level )
                continue;
/*
            fprintf( pFile, "  Node%d [label = \"%d\"", Gia_ObjId(p, pNode), Gia_ObjId(p, pNode) ); 
            if ( Gia_ObjIsXor(pNode) )
                fprintf( pFile, ", shape = doublecircle" );
            else if ( Gia_ObjIsMux(p, pNode) )
                fprintf( pFile, ", shape = trapezium" );
            else
                fprintf( pFile, ", shape = ellipse" );
*/
            if ( Vec_IntEntry(vMapAdds, iNode) >= 0 )
            {
                int iBox = Vec_IntEntry(vMapAdds, iNode);
                fprintf( pFile, "  Node%d [label = \"%d_%d\"", Gia_ShowAddOut(vAdds, vMapAdds, iNode), Vec_IntEntry(vAdds, 6*iBox+3), Vec_IntEntry(vAdds, 6*iBox+4) ); 
                if ( Vec_IntEntry(vAdds, 6*iBox+2) == 0 )
                    fprintf( pFile, ", shape = octagon" );
                else 
                    fprintf( pFile, ", shape = doubleoctagon" );
            }
            else if ( Vec_IntEntry(vMapXors, iNode) >= 0 )
            {
                fprintf( pFile, "  Node%d [label = \"%d\"", iNode, iNode ); 
                fprintf( pFile, ", shape = doublecircle" );
            }
            else if ( Gia_ObjIsXor(pNode) )
            {
                fprintf( pFile, "  Node%d [label = \"%d\"", iNode, iNode ); 
                fprintf( pFile, ", shape = doublecircle" );
            }
            else if ( Gia_ObjIsMux(p, pNode) )
            {
                fprintf( pFile, "  Node%d [label = \"%d\"", iNode, iNode ); 
                fprintf( pFile, ", shape = trapezium" );
            }
            else
            {
                fprintf( pFile, "  Node%d [label = \"%d\"", iNode, iNode ); 
                fprintf( pFile, ", shape = ellipse" );
            }
//            if ( pNode->fMark0 )
//                fprintf( pFile, ", style = filled" );
            fprintf( pFile, "];\n" );
        }
        fprintf( pFile, "}" );
        fprintf( pFile, "\n" );
        fprintf( pFile, "\n" );
    }

    // generate the CI nodes
    fprintf( pFile, "{\n" );
    fprintf( pFile, "  rank = same;\n" );
    // the labeling node of this level
    fprintf( pFile, "  Level%d;\n",  0 );
    // generate constant node
    if ( fConstIsUsed )
    {
        // check if the costant node is present
        fprintf( pFile, "  Node%d [label = \"Const0\"", 0 );
        fprintf( pFile, ", shape = ellipse" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    // generate the CI nodes
    Gia_ManForEachCi( p, pNode, i )
    {
        fprintf( pFile, "  Node%d [label = \"%d\"", Gia_ObjId(p, pNode), Gia_ObjId(p, pNode) ); 

        fprintf( pFile, ", shape = %s", "triangle" );
        fprintf( pFile, ", color = coral, fillcolor = coral" );
        fprintf( pFile, "];\n" );
    }
    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );

    // generate invisible edges from the square down
    fprintf( pFile, "title1 -> title2 [style = invis];\n" );
    Gia_ManForEachCo( p, pNode, i )
        fprintf( pFile, "title2 -> Node%d [style = invis];\n", Gia_ObjId(p, pNode) );
    // generate invisible edges among the COs
    Prev = -1;
    Gia_ManForEachCo( p, pNode, i )
    {
        if ( i > 0 )
            fprintf( pFile, "Node%d -> Node%d [style = invis];\n", Prev, Gia_ObjId(p, pNode) );
        Prev = Gia_ObjId(p, pNode);
    }
    // generate invisible edges among the CIs
    Prev = -1;
    Gia_ManForEachCi( p, pNode, i )
    {
        if ( i > 0 )
            fprintf( pFile, "Node%d -> Node%d [style = invis];\n", Prev, Gia_ObjId(p, pNode) );
        Prev = Gia_ObjId(p, pNode);
    }

    // generate edges
    Gia_ManForEachCo( p, pNode, i )
    {
        int iNode = Gia_ObjId( p, pNode );
        fprintf( pFile, "Node%d",  iNode );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Gia_ObjFaninId0(pNode, iNode)) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC0(pNode)? "dotted" : "bold" );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );
    }
    Gia_ManForEachObjVec( vOrder, p, pNode, i )
    {
        int iNode = Gia_ObjId( p, pNode );
//        if ( !Gia_ObjIsAnd(pNode) && !Gia_ObjIsCo(pNode) && !Gia_ObjIsBuf(pNode) )
//            continue;
        if ( Vec_IntEntry(vMapAdds, Gia_ObjId(p, pNode)) >= 0 )
        {
            int k, iBox = Vec_IntEntry(vMapAdds, iNode);
            for ( k = 0; k < 3; k++ )
                if ( Vec_IntEntry(vAdds, 6*iBox+k) )
                {
                    fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, iNode) );
                    fprintf( pFile, " -> " );
                    fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Vec_IntEntry(vAdds, 6*iBox+k)) );
                    fprintf( pFile, " [" );
                    fprintf( pFile, "style = %s", 0? "dotted" : "bold" );
                    fprintf( pFile, "]" );
                    fprintf( pFile, ";\n" );
                }
            continue;
        }
        if ( Vec_IntEntry(vMapXors, Gia_ObjId(p, pNode)) >= 0 )
        {
            int k, iXor = Vec_IntEntry(vMapXors, iNode);
            for ( k = 1; k < 4; k++ )
                if ( Vec_IntEntry(vXors, 4*iXor+k) )
                {
                    fprintf( pFile, "Node%d",  iNode );
                    fprintf( pFile, " -> " );
                    fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Vec_IntEntry(vXors, 4*iXor+k)) );
                    fprintf( pFile, " [" );
                    fprintf( pFile, "style = %s", 0? "dotted" : "bold" );
                    fprintf( pFile, "]" );
                    fprintf( pFile, ";\n" );
                }
            continue;
        }
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  iNode );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Gia_ObjFaninId0(pNode, iNode)) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC0(pNode)? "dotted" : "bold" );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );
        if ( !Gia_ObjIsAnd(pNode) )
            continue;
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  iNode );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Gia_ObjFaninId1(pNode, iNode)) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC1(pNode)? "dotted" : "bold" );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );

        if ( !Gia_ObjIsMux(p, pNode) )
            continue;
        // generate the edge from this node to the next
        fprintf( pFile, "Node%d",  iNode );
        fprintf( pFile, " -> " );
        fprintf( pFile, "Node%d",  Gia_ShowAddOut(vAdds, vMapAdds, Gia_ObjFaninId2(p, iNode)) );
        fprintf( pFile, " [" );
        fprintf( pFile, "style = %s", Gia_ObjFaninC2(p, pNode)? "dotted" : "bold" );
        fprintf( pFile, "]" );
        fprintf( pFile, ";\n" );
    }

    fprintf( pFile, "}" );
    fprintf( pFile, "\n" );
    fprintf( pFile, "\n" );
    fclose( pFile );

    Vec_IntFreeP( &p->vLevels );
}

/**Function*************************************************************

  Synopsis    [Returns DFS ordered array of objects and their levels.]

  Description []
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_ShowMapAdds( Gia_Man_t * p, Vec_Int_t * vAdds, int fFadds )
{
    Vec_Int_t * vMapAdds = Vec_IntStartFull( Gia_ManObjNum(p) ); int i;
    for ( i = 0; 6*i < Vec_IntSize(vAdds); i++ )
    {
        if ( fFadds && Vec_IntEntry(vAdds, 6*i+2) == 0 )
            continue;
        Vec_IntWriteEntry( vMapAdds, Vec_IntEntry(vAdds, 6*i+3), i );
        Vec_IntWriteEntry( vMapAdds, Vec_IntEntry(vAdds, 6*i+4), i );
    }
    return vMapAdds;
}
Vec_Int_t * Gia_ShowMapXors( Gia_Man_t * p, Vec_Int_t * vXors )
{
    Vec_Int_t * vMapXors = Vec_IntStartFull( Gia_ManObjNum(p) ); int i;
    for ( i = 0; 4*i < Vec_IntSize(vXors); i++ )
        Vec_IntWriteEntry( vMapXors, Vec_IntEntry(vXors, 4*i), i );
    return vMapXors;
}
int Gia_ShowCollectObjs_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vAdds, Vec_Int_t * vXors, Vec_Int_t * vMapAdds, Vec_Int_t * vMapXors, Vec_Int_t * vOrder )
{
    int Level0, Level1, Level2 = 0, Level = 0;
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return Gia_ObjLevel(p, pObj);
    Gia_ObjSetTravIdCurrent(p, pObj);
    if ( Gia_ObjIsCi(pObj) )
        return 0;
    if ( Vec_IntEntry(vMapAdds, Gia_ObjId(p, pObj)) >= 0 )
    {
        int iBox = Vec_IntEntry(vMapAdds, Gia_ObjId(p, pObj));
        Gia_ObjSetTravIdCurrentId(p, Vec_IntEntry(vAdds, 6*iBox+3) );
        Gia_ObjSetTravIdCurrentId(p, Vec_IntEntry(vAdds, 6*iBox+4) );
        Level0 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vAdds, 6*iBox+0) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level1 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vAdds, 6*iBox+1) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        if ( Vec_IntEntry(vAdds, 6*iBox+2) )
        Level2 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vAdds, 6*iBox+2) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level = 1 + Abc_MaxInt( Abc_MaxInt(Level0, Level1), Level2 );
        Gia_ObjSetLevelId( p, Vec_IntEntry(vAdds, 6*iBox+3), Level );
        Gia_ObjSetLevelId( p, Vec_IntEntry(vAdds, 6*iBox+4), Level );
        pObj = Gia_ManObj( p, Vec_IntEntry(vAdds, 6*iBox+4) );
    }
    else if ( Vec_IntEntry(vMapXors, Gia_ObjId(p, pObj)) >= 0 )
    {
        int iXor = Vec_IntEntry(vMapXors, Gia_ObjId(p, pObj));
        Level0 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vXors, 4*iXor+1) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level1 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vXors, 4*iXor+2) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        if ( Vec_IntEntry(vXors, 4*iXor+3) )
        Level2 = Gia_ShowCollectObjs_rec( p, Gia_ManObj( p, Vec_IntEntry(vXors, 4*iXor+3) ), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level = 1 + Abc_MaxInt( Abc_MaxInt(Level0, Level1), Level2 );
        Gia_ObjSetLevel( p, pObj, Level );
    }
    else
    {
        assert( !Gia_ObjIsMux(p, pObj) );
        Level0 = Gia_ShowCollectObjs_rec( p, Gia_ObjFanin0(pObj), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level1 = Gia_ShowCollectObjs_rec( p, Gia_ObjFanin1(pObj), vAdds, vXors, vMapAdds, vMapXors, vOrder );
        Level = 1 + Abc_MaxInt(Level0, Level1);
        Gia_ObjSetLevel( p, pObj, Level );
    }
    Vec_IntPush( vOrder, Gia_ObjId(p, pObj) );
    p->nLevels = Abc_MaxInt( p->nLevels, Level );
    return Level;
}
Vec_Int_t * Gia_ShowCollectObjs( Gia_Man_t * p, Vec_Int_t * vAdds, Vec_Int_t * vXors, Vec_Int_t * vMapAdds, Vec_Int_t * vMapXors )
{
    Gia_Obj_t * pObj; int i;
    Vec_Int_t * vOrder = Vec_IntAlloc( 100 );
    Gia_ManCleanLevels( p, Gia_ManObjNum(p) );
    p->nLevels = 0;
    Gia_ManIncrementTravId( p );
    Gia_ObjSetTravIdCurrent(p, Gia_ManConst0(p));
    Gia_ManForEachCi( p, pObj, i )
        Gia_ObjSetTravIdCurrent(p, pObj);
    Gia_ManForEachCo( p, pObj, i )
        Gia_ShowCollectObjs_rec( p, Gia_ObjFanin0(pObj), vAdds, vXors, vMapAdds, vMapXors, vOrder );
    return vOrder;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ShowProcess( Gia_Man_t * p, char * pFileName, Vec_Int_t * vAdds, Vec_Int_t * vXors, int fFadds )
{
    Vec_Int_t * vMapAdds = Gia_ShowMapAdds( p, vAdds, fFadds );
    Vec_Int_t * vMapXors = Gia_ShowMapXors( p, vXors );
    Vec_Int_t * vOrder = Gia_ShowCollectObjs( p, vAdds, vXors, vMapAdds, vMapXors );
    Gia_WriteDotAig( p, pFileName, vAdds, vXors, vMapAdds, vMapXors, vOrder );
    Vec_IntFree( vMapAdds );
    Vec_IntFree( vMapXors );
    Vec_IntFree( vOrder );
}
void Gia_ManShow( Gia_Man_t * pMan, Vec_Int_t * vBold, int fAdders, int fFadds )
{
    extern Vec_Int_t * Ree_ManComputeCuts( Gia_Man_t * p, Vec_Int_t ** pvXors, int fVerbose );
    extern void Ree_ManRemoveTrivial( Gia_Man_t * p, Vec_Int_t * vAdds );
    extern void Ree_ManRemoveContained( Gia_Man_t * p, Vec_Int_t * vAdds );

    extern void Abc_ShowFile( char * FileNameDot );
    static int Counter = 0;
    char FileNameDot[200];
    FILE * pFile;

    Vec_Int_t * vXors, * vAdds = Ree_ManComputeCuts( pMan, &vXors, 0 );
    Ree_ManRemoveTrivial( pMan, vAdds );
    Ree_ManRemoveContained( pMan, vAdds );

    // create the file name
//    Gia_ShowGetFileName( pMan->pName, FileNameDot );
    sprintf( FileNameDot, "temp%02d.dot", Counter++ );
    // check that the file can be opened
    if ( (pFile = fopen( FileNameDot, "w" )) == NULL )
    {
        fprintf( stdout, "Cannot open the intermediate file \"%s\".\n", FileNameDot );
        return;
    }
    fclose( pFile );
    // generate the file
    if ( fAdders )
        Gia_ShowProcess( pMan, FileNameDot, vAdds, vXors, fFadds );
    else
        Gia_WriteDotAigSimple( pMan, FileNameDot, vBold );
    // visualize the file 
    Abc_ShowFile( FileNameDot );

    Vec_IntFree( vAdds );
    Vec_IntFree( vXors );
}





////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END
