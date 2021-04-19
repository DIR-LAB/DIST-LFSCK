
#include <neo4j-client.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    neo4j_client_init();

//bolt://localhost:11005
//neo4j://user:pass@localhost:7687
//neo4j://neo4j:neo4j@localhost:7687

    /* use NEO4J_INSECURE when connecting to disable TLS */
    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:sk@localhost:11013", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        return EXIT_FAILURE;
    }







int inode_root = 0, inode_m1 = 1, inode_m2 =2;
int lma_root= 10, lma_m1 =20, lma_m2 = 30;
int link_root =0, link_m1 =10, link_m2=10;
int lov_root =0, lov_m1 =11, lov_m2 =12;

int inode_1 = 3, inode_2 =4;
int lma_1 =11, lma_2 = 12;
int fid1=20 , fid2=30;




/* ------------------ to use sprintf --------------------------------------*/
char buffer[100];




clock_t begin = clock();
/*-----------------------------------root mds-----------------------------------*/
sprintf(buffer, "CREATE (:MDS {Inode_num: %d,lma:%d,linkEA:%d,lovEA:%d});",inode_root, lma_root, link_root, lov_root);
neo4j_result_stream_t *results =
neo4j_run(connection, buffer, neo4j_null);
neo4j_close_results(results);
/*------------------------------------------------------------------------------*/

/*-------------------------------------------------child mds---------------------------------------------------------------*/
sprintf(buffer, "CREATE (:MDS {Inode_num: %d,lma:%d,linkEA:%d,lovEA:%d});",inode_m1, lma_m1, link_m1, lov_m1);
results = neo4j_run(connection, buffer, neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:MDS) where a.linkEA=b.lma CREATE (b)-[r:parent_of]->(a);" , neo4j_null);
neo4j_close_results(results);
/*------------------------------------------------------------------------------------------------------------------------*/
sprintf(buffer, "CREATE (:MDS {Inode_num: %d,lma:%d,linkEA:%d,lovEA:%d});",inode_m2, lma_m2, link_m2, lov_m2);
results = neo4j_run(connection, buffer, neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:MDS) where a.linkEA=b.lma merge (b)-[r:parent_of]->(a);" , neo4j_null);
neo4j_close_results(results);
/*------------------------------------------------------------------------------------------------------------------------*/






/*-------------------------------------------------oss nodes---------------------------------------------------------------*/
sprintf(buffer, "CREATE (:OSS {Inode_num: %d,lma:%d,FID:%d});",inode_1, lma_1, fid1);
results = neo4j_run(connection, buffer, neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:OSS) where a.lovEA=b.lma CREATE (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:OSS) where a.lma=b.FID CREATE (b)-[:oss_child_of]->(a);" , neo4j_null);
neo4j_close_results(results);
/*------------------------------------------------------------------------------------------------------------------------*/
sprintf(buffer, "CREATE (:OSS {Inode_num: %d,lma:%d,FID:%d});",inode_2, lma_2, fid2);
results = neo4j_run(connection, buffer, neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:OSS) where a.lovEA=b.lma merge (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:MDS), (b:OSS) where a.lma=b.FID merge (b)-[:oss_child_of]->(a);" , neo4j_null);
neo4j_close_results(results);
/*------------------------------------------------------------------------------------------------------------------------*/

clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
printf("%f seconds \n", time_spent);


    neo4j_close(connection);
    neo4j_client_cleanup();
    return EXIT_SUCCESS;
}








/*-----------------------------------------------------without sprintf----------------------------------------------------*/
/* //root mds
//neo4j_run(connection, "CREATE (:mds {Inode_num: 0,lma:10,linkEA:0,lovEA:0});", neo4j_null);

//child mds
neo4j_run(connection, "CREATE (:mds {Inode_num: 1,lma:20,linkEA:10,lovEA:11});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:mds) where a.linkEA=b.lma CREATE (b)-[r:parent_of]->(a);" , neo4j_null);
neo4j_run(connection, "CREATE (:mds {Inode_num: 2,lma:30,linkEA:10,lovEA:12});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:mds) where a.linkEA=b.lma CREATE (b)-[r:parent_of]->(a);" , neo4j_null);

//oss
neo4j_run(connection, "CREATE (:oss {Inode_num: 3,lma:11,FID:20});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lovEA=b.lma CREATE (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lma=b.FID CREATE (b)-[:oss_child_of]->(a);" , neo4j_null);
neo4j_run(connection, "CREATE (:oss {Inode_num: 4,lma:12,FID:30});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lovEA=b.lma CREATE (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lma=b.FID CREATE (b)-[:oss_child_of]->(a);" , neo4j_null);
*/
/*------------------------------------------------------------------------------------------------------------------------*/





		
/*	
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to run statement");
        return EXIT_FAILURE;
    }
*/
   





/*
    neo4j_result_t *result = neo4j_fetch_next(results);
    if (result == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        return EXIT_FAILURE;
    }

    neo4j_value_t value = neo4j_result_field(result, 0);
    char buf[128];
    printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));




    //neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();
    return EXIT_SUCCESS;
}
*/








/*create (:MDS {inode_num: 1, lma: 2})
match (a:MDS)
where a.inode-num = '1'
return a;


match (a {inode_num: 1})
return a;
*/

/* works:
neo4j_run(connection, "match (c:MDS {Inode_num: 1,lma:10,linkEA:0,lovEA:0}) delete c;" , neo4j_null);


match (a:MEh), (b:MEh)
where a.linkEA=b.lma
CREATE (a)-[r:parent_of]->(b)

*/

/* ---- to delete all nodes (same type)
MATCH (n:OSS) 
WITH n LIMIT 5
DETACH DELETE n
RETURN count(*);
*/












