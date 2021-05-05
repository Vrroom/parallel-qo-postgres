/*
 * Queries taken from: 
 *   1. https://github.com/hpausiello/SQL-pagila-queries 
 *   2. https://github.com/joelsotelods/sakila-db-queries
 */
select count(*) from actor, film_actor;
select count(*) from actor, film_actor, film, inventory where actor.actor_id = film_actor.actor_id and film_actor.film_id = film.film_id and film.film_id = inventory.film_id;
select count(*) from actor, film_actor, film where actor.actor_id = film_actor.actor_id and film_actor.film_id = film.film_id;
select first_name, last_name from actor;
select * from actor l left join film_actor r on l.actor_id = r.actor_id;
select * from actor l inner join film_actor r on l.actor_id = r.actor_id;
select * from actor l right join film_actor r on l.actor_id = r.actor_id;
select s.first_name, s.last_name, a.address from staff s left join address a on s.address_id = a.address_id;
select f.title, count(i.inventory_id) as number_of_copies from film f join inventory i on f.film_id = i.film_id where f.title = 'HUNCHBACK IMPOSSIBLE' group by  f.title;
select s.first_name from (select a.first_name, a.last_name, f.title from actor a join film_actor fa on a.actor_id = fa.actor_id join film f on f.film_id = fa.film_id where title = 'alone trip' ) s group by s.first_name, s.last_name order by  s.last_name asc;
select c.name, sum(p.amount) as gross_revenue from category c join film_category fc on c.category_id = fc.category_id join inventory i on fc.film_id = i.film_id join rental r on i.inventory_id = r.inventory_id join payment p on r.rental_id = p.rental_id group by  c.name order by  gross_revenue desc limit 5;
select A.*, B.sales
from (
	select sto.store_id, cit.city, cou.country
	from store sto
	left join address adr
	on sto.address_id = adr.address_id
	join city cit
	on adr.city_id = cit.city_id
	join country cou
	on cit.country_id = cou.country_id
) A
join (
	select cus.store_id, sum(pay.amount) sales
	from customer cus
	join payment pay
	on pay.customer_id = cus.customer_id
	group by cus.store_id
) B
on A.store_id = B.store_id
order by a.store_id;
select cat.name category_name, sum( coalesce(pay.amount, 0) ) revenue
from category cat
left join film_category flm_cat
on cat.category_id = flm_cat.category_id
left join film fil
on flm_cat.film_id = fil.film_id
left join inventory inv
on fil.film_id = inv.film_id
left join rental ren
on inv.inventory_id = ren.inventory_id
left join payment pay
on ren.rental_id = pay.rental_id
group by cat.name
order by revenue desc
limit 5;
