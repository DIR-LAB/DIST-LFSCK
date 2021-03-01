foreach inode in mds_local_storage:  // scan each inode in MDS server
	// create the MDS Object using given inodeId and fileID
	CREATE (a:MDSOBJECT {InodeID: inode.id, FileID: inode.ea.LMA})

	// there are some checkings on users and groups, so we include them
	MERGE (user:USER {UID: inode.uid})
	MERGE (group:GROUP {GID: inode.gid})
	// create relationships from the mds object to corresponding user and group
	CREATE (a)-[r:belong_to_user]->(user)
	CREATE (a)-[r:belong_to_group]->(group)

	// match the parent object in MDS and create a link (LinkEA)
	MATCH (parent:MDSOBJECT) where parent.FileID = inode.P_FileID 
	CREATE (a)-[r:child_of]->(parent)

	// if this inode is a directory, connect it with all its children
	if inode.is_directory():
		foreach child_file_id in inode.dir_entry:
			MATCH (child:MDSOBJECT) where child.FileID = child_file_id
			CREATE (a)-[r:parent_of]->(child)
	// if it is a file, then connect it with all its stripes.
	else:
		// use ost_id and object_id to locate an unique ost object
		// I am not sure whether ost object id is inode ID
		foreach <ost_id, object_id> in inode.lov_ea:
			MERGE (o:OSTOBJECT {OSTID: ost_id, InodeID: object_id})
			CREATE (a)-[r:parent_of_stripe]->(o)
			
// scan each inode in OST servers
foreach inode in ost_local_storage:
	// locate the OST object, which might have been created before
	MERGE (o:OSTOBJECT {OSTID: local_ost_id, InodeID: inode.id})
	// connect it with corresponding user and group
	MERGE (user:USER {UID: inode.uid})
	MERGE (group:GROUP (GID: inode.gid})
	CREATE (o)-[r:belong_to_user]->(user)
	CREATE (o)-[r:belong_to_group]->(group)
	
	// connect it back to the MDS object with the same FileID
	MERGE (a:MDSOBJECT {FileID: inode.ea.LMA})
	CREATE (o)-[r:stripe_of_parent]->(a)
