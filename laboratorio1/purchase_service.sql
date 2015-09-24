create table requests (
        id 				integer primary key autoincrement not null,
        customer_id		integer not null,
        transaction_id	varchar(20) not null,
        card_number		varchar(20) not null,
        amount			integer not null,
        status 			integer default 0 not null, 
        receive_date 	timestamp default current_timestamp not null
);

create table cards(
				card_number varchar(20) primary key,
				cupo integer not null,
				correo varchar(40) not null
);

insert into cards card_number,cupo,correo values('9876789545A',1000000,'arevaloarboled@hotmail.com');